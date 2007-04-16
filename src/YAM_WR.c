/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2007 by YAM Open Source Team

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 YAM Official Support Site :  http://www.yam.ch/
 YAM Open Source project   :  http://sourceforge.net/projects/yamos/

 $Id$

***************************************************************************/

/***************************************************************************
 Module: Write
***************************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <clib/alib_protos.h>
#include <libraries/asl.h>
#include <libraries/gadtools.h>
#include <libraries/iffparse.h>
#include <mui/BetterString_mcc.h>
#include <mui/NList_mcc.h>
#include <mui/NListtree_mcc.h>
#include <mui/NListview_mcc.h>
#include <mui/TextEditor_mcc.h>
#include <workbench/workbench.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/muimaster.h>
#include <proto/utility.h>

#include "extrasrc.h"

#include "SDI_hook.h"

#include "YAM.h"
#include "YAM_addressbook.h"
#include "YAM_addressbookEntry.h"
#include "YAM_config.h"
#include "YAM_configGUI.h"
#include "YAM_error.h"
#include "YAM_folderconfig.h"
#include "YAM_global.h"
#include "YAM_glossarydisplay.h"
#include "YAM_locale.h"
#include "YAM_main.h"
#include "YAM_mainFolder.h"
#include "YAM_mime.h"
#include "YAM_read.h"
#include "YAM_utilities.h"
#include "YAM_write.h"
#include "classes/Classes.h"

#include "Debug.h"

/**************************************************************************/
/* local protos */
static struct WR_ClassData *WR_NewBounce(int);
static struct WR_ClassData *WR_New(int winnum);

/**************************************************************************/

/*** Attachments list ***/
/// WR_GetFileEntry
/*** WR_GetFileEntry -Fills form with data from selected list entry ***/
HOOKPROTONHNO(WR_GetFileEntry, void, int *arg)
{
   int winnum = *arg;
   struct Attach *attach = NULL;
   struct WR_GUIData *gui = &G->WR[winnum]->GUI;

   DoMethod(gui->LV_ATTACH, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &attach);
   DoMethod(G->App, MUIM_MultiSet, MUIA_Disabled, attach ? FALSE : TRUE, gui->RA_ENCODING, gui->ST_CTYPE, gui->ST_DESC, gui->BT_DEL, gui->BT_DISPLAY, NULL);
   if (attach)
   {
      nnset(gui->RA_ENCODING, MUIA_Radio_Active, attach->IsMIME ? 0 : 1);
      nnset(gui->ST_CTYPE, MUIA_String_Contents, attach->ContentType);
      nnset(gui->ST_DESC, MUIA_String_Contents, attach->Description);
   }
}
MakeStaticHook(WR_GetFileEntryHook, WR_GetFileEntry);

///
/// WR_PutFileEntry
/*** WR_PutFileEntry - Fills form data into selected list entry ***/
HOOKPROTONHNO(WR_PutFileEntry, void, int *arg)
{
   int winnum = *arg;
   struct Attach *attach = NULL;
   struct WR_GUIData *gui = &G->WR[winnum]->GUI;

   DoMethod(gui->LV_ATTACH, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &attach);
   if (attach)
   {
      int ismime = xget(gui->RA_ENCODING, MUIA_Radio_Active);

      attach->IsMIME = (ismime == 0);
      GetMUIString(attach->ContentType, gui->ST_CTYPE, sizeof(attach->ContentType));
      GetMUIString(attach->Description, gui->ST_DESC, sizeof(attach->Description));
      DoMethod(gui->LV_ATTACH, MUIM_NList_Redraw, MUIV_NList_Redraw_Active);
   }
}
MakeStaticHook(WR_PutFileEntryHook, WR_PutFileEntry);

///
/// WR_AddFileToList
/*** WR_AddFileToList - Adds a file to the attachment list, gets its size and type ***/
BOOL WR_AddFileToList(int winnum, const char *filename, const char *name, BOOL istemp)
{
  struct WR_GUIData *gui = &G->WR[winnum]->GUI;
  BOOL result = FALSE;

  ENTER();

  if(filename)
  {
    static struct Attach attach;
    const char *ctype;
    BPTR lock;

    memset(&attach, 0, sizeof(struct Attach));

    // we try to get the filesize and comment and copy
    // it into our attchement structure.
    if((lock = Lock((STRPTR)filename, ACCESS_READ)))
    {
      struct FileInfoBlock *fib;

      if((fib = AllocDosObject(DOS_FIB, NULL)))
      {
        if(Examine(lock, fib))
        {
          attach.Size = fib->fib_Size;
          strlcpy(attach.Description, fib->fib_Comment, sizeof(attach.Description));

          // check the dir type and protection
          if(fib->fib_DirEntryType < 0 &&
             isFlagClear(fib->fib_Protection, FIBF_READ))
          {
            result = TRUE;
          }
        }

        FreeDosObject(DOS_FIB, fib);
      }

      UnLock(lock);
    }

    // now we try to find out the content-type of the
    // attachment by using the IdentifyFile() function which will
    // do certain checks to guess the content-type out of the file name
    // and content.
    if(result == TRUE && (ctype = IdentifyFile(filename)) && ctype[0] != '\0')
    {
      int encoding = xget(gui->RA_ENCODING, MUIA_Radio_Active);

      attach.IsMIME = (encoding == 0);
      attach.IsTemp = istemp;
      strlcpy(attach.FilePath, filename, sizeof(attach.FilePath));
      strlcpy(attach.Name, name ? name : (char *)FilePart(filename), sizeof(attach.Name));
      strlcpy(attach.ContentType, ctype, sizeof(attach.ContentType));
      nnset(gui->ST_CTYPE, MUIA_String_Contents, attach.ContentType);
      nnset(gui->ST_DESC, MUIA_String_Contents, attach.Description);

      // put the attachment into our attachment MUI list and set
      // it active.
      DoMethod(gui->LV_ATTACH, MUIM_NList_InsertSingle, &attach, MUIV_NList_Insert_Bottom);
      set(gui->LV_ATTACH, MUIA_NList_Active, MUIV_NList_Active_Bottom);
    }
    else
    {
      ER_NewError(tr(MSG_ER_ADDFILEERROR), filename);

      result = FALSE;
    }
  }

  RETURN(result);
  return result;
}
///

/*** Compose Message ***/
/// GetDateTime
//  Formats current date and time for Date header field
static char *GetDateTime(void)
{
  static char dt[SIZE_DEFAULT];
  DateStamp2RFCString(dt, sizeof(dt), NULL, C->TimeZone + (C->DaylightSaving ? 60 : 0), FALSE);
  return dt;
}

///
/// NewID
//  Creates a unique id, used for Message-ID header field
static char *NewID(BOOL is_msgid)
{
   static char idbuf[SIZE_MSGID];

   if(is_msgid)
   {
      ULONG seconds;
      struct DateStamp ds;

      // lets calculate the seconds
      DateStamp(&ds);
      seconds = ds.ds_Days*24*60*60+ds.ds_Minute*60; // seconds since 1-Jan-78

      // Here we try to generate a unique MessageID.
      // We try to be as much conform to the Recommandations for generating
      // unqiue Message IDs as we can: http://www.jwz.org/doc/mid.html
      snprintf(idbuf, sizeof(idbuf), "%lx%lx.%lx@%s", seconds, ds.ds_Tick, (ULONG)rand(), C->SMTP_Server);
   }
   else
   {
      static int ctr = 0;

      // Generate a unique Boundary ID which conforms to RFC 2045 and includes
      // a "=_" sequence to make it safe for quoted printable encoded parts
      snprintf(idbuf, sizeof(idbuf), "--=_BOUNDARY.%lx%lx.%02x", (ULONG)FindTask(NULL), (ULONG)rand(), ++ctr);
   }

   return idbuf;
}

///
/// WhichEncodingForFile
//  Determines best MIME encoding mode for a file
static enum Encoding WhichEncodingForFile(const char *fname, const char *ctype)
{
   int c, linesize=0, total=0, unsafechars=0, binarychars=0, longlines=0;
   FILE *fh = fopen(fname, "r");

   if(!fh)
     return ENC_B64;

   setvbuf(fh, NULL, _IOFBF, SIZE_FILEBUF);

   // scan until end of file
   while((c = fgetc(fh)) != EOF)
   {
      linesize++; // count the characters to get linelength
      total++;    // count the total number of scanned characters

      // first we check if this is a linebreak
      if(c == '\n')
      {
         // (RFC 821) restricts 7bit lines to a maximum of 1000 characters
         // so we have to use QP or base64 later on.
         // but RFC 2822 says that lines shouldn`t be longer than 998 chars, so we take this one
         if(linesize > 998) ++longlines;
         linesize = 0;
      }
      else if (c > 127) ++unsafechars; // count the number of unprintable >7bit characters
      else if (c < 32 && c != '\t' && c != '\r') ++binarychars; // count the number of chars used in binaries.

      // if we successfully scanned 4000 bytes out of the file and found enough
      // data we break out here. we have to at least find some longlines or
      // we have to scan the whole part.
      if(total > 4000 && longlines) break;
   }
   fclose(fh);

   D(DBF_MIME, "EncodingTest [%s] t:%ld l:%ld u:%ld b:%ld", fname, total, longlines, unsafechars, binarychars);

   // now that we analyzed the file we have to decide which encoding to take
   if(longlines || unsafechars || binarychars)
   {
      // we make sure that the following content-types get always encoded via base64
      if(!strnicmp(ctype, "image/", 6) ||
         !strnicmp(ctype, "audio/", 6) ||
         !strnicmp(ctype, "video/", 6))      return ENC_B64;

      // if we are here just because of long lines we have to decide for either
      // binary or quoted-printable encoding.
      if(!unsafechars && !binarychars)       return C->Allow8bit ? ENC_BIN : ENC_QP;

      // if there are no binary chars in the file we just have
      // unsafe 8bit characters and if the server support them we can easily decide between binary or 8bit
      if(!binarychars && C->Allow8bit)       return longlines ? ENC_BIN : ENC_8BIT;

      // if we end up here we have a file with just unprintable characters
      // and we have to decide if we take base64 or quoted-printable.
      // base64 is more compact if there are many unprintable characters, as
      // when sending a graphics file or such. see (RFC 1521)
      if(total/(unsafechars+binarychars+1) < 16 || !strnicmp(ctype, "application/", 12))
      {
        return ENC_B64;
      }
      else
      {
        return ENC_QP;
      }
   }

   // if there are no special stuff within the file we can break out
   // telling the caller that there is no encoding needed.
   return ENC_NONE;
}

///
/// NewPart
//  Initializes a new message part
struct WritePart *NewPart(int winnum)
{
  struct WritePart *p;

  ENTER();

  if((p = calloc(1, sizeof(struct WritePart))) != NULL)
  {
    p->ContentType = "text/plain";
    p->EncType = ENC_NONE;
    p->Filename = G->WR_Filename[winnum];
  }

  RETURN(p);
  return p;
}

///
/// BuildPartsList
//  Builds message parts from attachment list
static struct WritePart *BuildPartsList(int winnum)
{
  struct WritePart *first;

  ENTER();

  if((first = NewPart(winnum)) != NULL)
  {
    int i;
    struct WritePart *p;

    p = first;
    p->IsTemp = TRUE;
    p->EncType = WhichEncodingForFile(p->Filename, p->ContentType);

    for(i = 0; ; i++)
    {
      struct Attach *att;
      struct WritePart *np;

      DoMethod(G->WR[winnum]->GUI.LV_ATTACH, MUIM_NList_GetEntry, i, &att);
      if(att == NULL)
        break;

      if((np = NewPart(winnum)) != NULL)
      {
        p->Next = np;
        p->ContentType = att->ContentType;
        p->Filename    = att->FilePath;
        p->Description = att->Description;
        p->Name        = att->Name;
        p->IsTemp      = att->IsTemp;
        if(att->IsMIME)
          p->EncType = WhichEncodingForFile(p->Filename, p->ContentType);
        else
          p->EncType = ENC_UUE;

        p = np;
      }
    }
  }

  RETURN(first);
  return first;
}

///
/// FreePartsList
//  Clears message parts and deletes temporary files
void FreePartsList(struct WritePart *p)
{
  struct WritePart *np;

  ENTER();

  for(; p; p = np)
  {
    np = p->Next;
    if(p->IsTemp)
      DeleteFile(p->Filename);

    free(p);
  }

  LEAVE();
}

///
/// HeaderFputs
// Outputs the value of a header line directly to the FILE pointer and handle
// RFC2231 complicant MIME parameter encoding, but also RFC2047 compliant
// MIME value encoding as well. As soon as "param" is set to NULL, RFC2047
// encoding will be used, otherwise RFC2231-based one.
static void HeaderFputs(FILE *fh, const char *s, const char *param)
{
  BOOL doEncoding = FALSE;
  char *c = (char *)s;
  int paramLen = 0;

  ENTER();

  if(param)
    paramLen = strlen(param);

  // let us now search for any non-ascii compliant character aswell
  // as converting each character with the translation table
  while(*c)
  {
    // check for any non-ascii character or
    // if the string would be
    if(!isascii(*c) || iscntrl(*c) || *c == '"')
    {
      doEncoding = TRUE;
      break;
    }

    // increment the pointer
    c++;
  }

  // in case we want to process a MIME parameter we have to
  // check other things as well.
  if(doEncoding == FALSE)
  {
    if(param != NULL)
      doEncoding = (c-s+1+paramLen+6) > 78;
    else
    {
      // we have to check for stray =? strings as we are
      // going to consider a rfc2047 encoding
      doEncoding = ((c = strstr(s, "=?")) && isascii(*(c+1)) &&
                    (c == s || isspace(*(c-1))));

    }

  }


  // if an encoding is required, we go and process it accordingly but
  // have to check whether we do rfc2047 or rfc2231 based encoding
  if(doEncoding)
  {
    if(param)
    {
      // do the actual rfc2231 based MIME paramater encoding
      rfc2231_encode_file(fh, param, s);
    }
    else
    {
      // do the actual rfc2047 based encoding
      rfc2047_encode_file(fh, s);
    }
  }
  else if(param)
  {
    // output the parameter name right before
    // the resulting parameter value
    fprintf(fh, "\n\t%s=\"%s\"", param, s);
  }
  else
  {
    // there seems to be non "violating" characters in the string and
    // the resulting string will also be not > 78 chars in case we
    // have to encode a MIME parameter, so we go and putout the source
    // string immediately
    fputs(s, fh);
  }

  LEAVE();
}

///
/// EmitHeader
//  Outputs a complete header line
void EmitHeader(FILE *fh, const char *hdr, const char *body)
{
  fprintf(fh, "%s: ", hdr);
  HeaderFputs(fh, body, NULL);
  fputc('\n', fh);
}

///
/// EmitRcptField
//  Outputs the value of a recipient header line, one entry per line
static void EmitRcptField(FILE *fh, const char *body)
{
  char *bodycpy;

  if((bodycpy = strdup(body)))
  {
    char *part = bodycpy;

    while(part)
    {
      char *next;

      if(*part == '\0')
        break;

      if((next = MyStrChr(part, ',')))
        *next++ = '\0';

      HeaderFputs(fh, Trim(part), NULL);

      if((part = next))
        fputs(",\n\t", fh);
    }

    free(bodycpy);
  }
}

///
/// EmitRcptHeader
//  Outputs a complete recipient header line
static void EmitRcptHeader(FILE *fh, const char *hdr, const char *body)
{
  fprintf(fh, "%s: ", hdr);
  EmitRcptField(fh, body ? body : "");
  fputc('\n', fh);
}

///
/// WriteContentTypeAndEncoding
//  Outputs content type header including parameters
static void WriteContentTypeAndEncoding(FILE *fh, struct WritePart *part)
{
  char *p;

  ENTER();

  // output the "Content-Type:
  fprintf(fh, "Content-Type: %s", part->ContentType);
  if(part->EncType != ENC_NONE && strncmp(part->ContentType, "text/", 5) == 0)
    fprintf(fh, "; charset=%s", strippedCharsetName(G->localCharset));

  // output the "name" and Content-Disposition as well
  // as the "filename" parameter to the mail
  if((p = part->Name) && *p)
  {
    fputc(';', fh);
    HeaderFputs(fh, p, "name"); // output and do rfc2231 encoding
    fputs("\nContent-Disposition: attachment;", fh);
    HeaderFputs(fh, p, "filename"); // output and do rfc2231 encoding
  }
  fputc('\n', fh);

  // output the Content-Transfer-Encoding:
  if(part->EncType != ENC_NONE)
  {
    const char *enc = NULL;

    switch(part->EncType)
    {
      case ENC_B64:  enc = "base64"; break;
      case ENC_QP:   enc = "quoted-printable"; break;
      case ENC_UUE:  enc = "x-uue"; break;
      case ENC_8BIT: enc = "8bit"; break;
      case ENC_BIN:  enc = "binary"; break;
      case ENC_NONE:
        // nothing
      break;
    }

    fprintf(fh, "Content-Transfer-Encoding: %s\n", enc ? enc : "7bit");
  }

  // output the Content-Description if appropriate
  if((p = part->Description) && *p)
    EmitHeader(fh, "Content-Description", p);

  LEAVE();
}

///
/// WR_WriteUserInfo
//  Outputs X-SenderInfo header line
static void WR_WriteUserInfo(FILE *fh, char *from)
{
  struct ABEntry *ab = NULL;
  struct Person pers = { "", "" };

  // Now we extract the real email from the address string
  if(*from)
    ExtractAddress(from, &pers);

  if(*(pers.Address) && AB_SearchEntry(pers.Address, ASM_ADDRESS|ASM_USER, &ab))
  {
    if(ab->Type != AET_USER)
      ab = NULL;
    else if(!*ab->Homepage && !*ab->Phone && !*ab->Street && !*ab->City && !*ab->Country && !ab->BirthDay)
      ab = NULL;
  }

  if(!ab && !*C->MyPictureURL)
    return;

  fputs("X-SenderInfo: 1", fh);
  if(*C->MyPictureURL) { fputc(';', fh); HeaderFputs(fh, C->MyPictureURL, "picture"); }

  if(ab)
  {
    if(*ab->Homepage) { fputc(';', fh); HeaderFputs(fh, ab->Homepage, "homepage"); }
    if(*ab->Street)   { fputc(';', fh); HeaderFputs(fh, ab->Street, "street"); }
    if(*ab->City)     { fputc(';', fh); HeaderFputs(fh, ab->City, "city"); }
    if(*ab->Country)  { fputc(';', fh); HeaderFputs(fh, ab->Country, "country"); }
    if(*ab->Phone)    { fputc(';', fh); HeaderFputs(fh, ab->Phone, "phone"); }
    if(ab->BirthDay)  fprintf(fh, ";\n\tdob=%ld", ab->BirthDay);
  }
  fputc('\n', fh);
}
///
/// EncodePart
//  Encodes a message part
static void EncodePart(FILE *ofh, struct WritePart *part)
{
   FILE *ifh;

   ENTER();

   if((ifh = fopen(part->Filename, "r")))
   {
      setvbuf(ifh, NULL, _IOFBF, SIZE_FILEBUF);

      switch(part->EncType)
      {
         case ENC_B64:
         {
            BOOL convLF = FALSE;

            // let us first check if we need to convert single LF to
            // CRLF to be somewhat portable.
            if(!strnicmp(part->ContentType, "text", 4)    ||
               !strnicmp(part->ContentType, "message", 7) ||
               !strnicmp(part->ContentType, "multipart", 9))
            {
              convLF = TRUE;
            }

            // then start base64 encoding the whole file.
            if(base64encode_file(ifh, ofh, convLF) <= 0)
            {
              ER_NewError(tr(MSG_ER_B64FILEENCODE), part->Filename);
            }
         }
         break;

         case ENC_QP:
         {
            if(qpencode_file(ifh, ofh) < 0)
            {
              ER_NewError(tr(MSG_ER_QPFILEENCODE), part->Filename);
            }
         }
         break;

         case ENC_UUE:
         {
            int size = FileSize(part->Filename);

            fprintf(ofh, "begin 644 %s\n", *part->Name ? part->Name : (char *)FilePart(part->Filename));

            if(uuencode_file(ifh, ofh) < 0)
            {
              ER_NewError(tr(MSG_ER_UUFILEENCODE), part->Filename);
            }

            fprintf(ofh, "``\nend\nsize %d\n", size);
         }
         break;

         default:
         {
            CopyFile(NULL, ofh, NULL, ifh);
         }
      }

      fclose(ifh);
   }

   LEAVE();
}

///
/// WR_CreateHashTable
//  Creates an index table for a database file
static BOOL WR_CreateHashTable(char *source, char *hashfile, char *sep)
{
  BOOL result = FALSE;
  FILE *in;

  ENTER();

  if((in = fopen(source, "r")) != NULL)
  {
    FILE *out;

    setvbuf(in, NULL, _IOFBF, SIZE_FILEBUF);

    if((out = fopen(hashfile, "w")) != NULL)
    {
      char buffer[SIZE_LARGE];
      const size_t l = strlen(sep);

      setvbuf(out, NULL, _IOFBF, SIZE_FILEBUF);

      // the first offset is always zero
      WriteUInt32(out, 0);
      while(fgets(buffer, sizeof(buffer), in))
      {
        // if we found a separator write out the current offset
        if(strncmp(buffer, sep, l) == 0)
          WriteUInt32(out, ftell(in));
      }

      result = TRUE;

      fclose(out);
    }

    fclose(in);
  }

  RETURN(result);
  return result;
}

///
/// WR_AddTagline
//  Randomly selects a tagline and writes it to the message file
static void WR_AddTagline(FILE *fh_mail)
{
  ENTER();

  if(C->TagsFile[0] != '\0')
  {
    char hashfile[SIZE_PATHFILE];
    FILE *fh_tag;

    snprintf(hashfile, sizeof(hashfile), "%s.hsh", C->TagsFile);

    if(FileTime(C->TagsFile) > FileTime(hashfile))
      WR_CreateHashTable(C->TagsFile, hashfile, C->TagsSeparator);

    if((fh_tag = fopen(C->TagsFile, "r")) != NULL)
    {
      FILE *fh_hash;

      setvbuf(fh_tag, NULL, _IOFBF, SIZE_FILEBUF);

      if((fh_hash = fopen(hashfile, "r")) != NULL)
      {
        long hsize;

        setvbuf(fh_hash, NULL, _IOFBF, SIZE_FILEBUF);

        fseek(fh_hash, 0, SEEK_END);
        hsize = ftell(fh_hash);
        if((hsize = hsize / sizeof(long)) > 1)
        {
          long fpos;

          // calculate a random offset
          fpos = (((long)rand()) % hsize) * sizeof(long);
          fseek(fh_hash, fpos, SEEK_SET);
          // read the offset to the tagline from the hash file
          if(ReadUInt32(fh_hash, (ULONG *)&fpos) == 1)
          {
            char buf[SIZE_LARGE];

            fseek(fh_tag, fpos, SEEK_SET);

            if(GetLine(fh_tag, buf, sizeof(buf)) != NULL)
            {
              fputs(buf, fh_mail);

              while(GetLine(fh_tag, buf, sizeof(buf)) != NULL)
              {
                if(strncmp(buf, C->TagsSeparator, strlen(C->TagsSeparator)) == 0)
                  break;
                else
                  fprintf(fh_mail, "\n%s", buf);
              }
            }
          }
        }

        fclose(fh_tag);
      }
      else
        ER_NewError(tr(MSG_ER_CantOpenFile), hashfile);
    }
    else
      ER_NewError(tr(MSG_ER_CantOpenFile), C->TagsFile);
  }

  LEAVE();
}

///
/// WR_WriteSignature
//  Writes signature to the message file
static void WR_WriteSignature(FILE *out, int signat)
{
  FILE *in;

  ENTER();

  if((in = fopen(CreateFilename(SigNames[signat]), "r")))
  {
    int ch;

    setvbuf(in, NULL, _IOFBF, SIZE_FILEBUF);

    fputs("-- \n", out);
    while((ch = fgetc(in)) != EOF)
    {
      if(ch == '%')
      {
        ch = fgetc(in);

        if(ch == 't')
        {
          WR_AddTagline(out);
          continue;
        }

        if(ch == 'e')
        {
          CopyFile(NULL, out, "ENV:SIGNATURE", NULL);
          continue;
        }

        ungetc(ch, in);
        ch = '%';
      }
      fputc(ch, out);
    }

    fclose(in);
  }

  LEAVE();
}

///
/// WR_AddSignature
//  Adds a signature to the end of the file
void WR_AddSignature(int winnum, int signat)
{
   char *mailfile = G->WR_Filename[winnum];

   if (signat == -1)
      signat = C->UseSignature ? 1 : 0;

   if (signat)
   {
      FILE *fh_mail;
      BOOL addline = FALSE;

      if ((fh_mail = fopen(mailfile, "r")))
      {
         fseek(fh_mail, -1, SEEK_END);
         addline = fgetc(fh_mail) != '\n';
         fclose(fh_mail);
      }

      if((fh_mail = fopen(mailfile, "a")))
      {
         setvbuf(fh_mail, NULL, _IOFBF, SIZE_FILEBUF);

         if (addline)
            fputc('\n', fh_mail);

         WR_WriteSignature(fh_mail, signat-1);
         fclose(fh_mail);
      }
   }

   // lets set the signature radiobutton
   // accordingly to the set signature
   nnset(G->WR[winnum]->GUI.RA_SIGNATURE, MUIA_Radio_Active, signat);
}

///
/// WR_Anonymize
//  Inserts recipient header field for remailer service
static void WR_Anonymize(FILE *fh, char *body)
{
   char *ptr;

   for (ptr = C->RMCommands; *ptr; ptr++)
   {
      if (*ptr == '\\') if (*(ptr+1) == 'n') { ptr++; fputs("\n", fh); continue; }
      if (*ptr == '%') if (*(ptr+1) == 's') { ptr++; EmitRcptField(fh, body); continue; }
      fputc(*ptr, fh);
   }
   fputs("\n", fh);
}

///
/// WR_GetPGPId
//  Gets PGP key id for a person
static char *WR_GetPGPId(struct Person *pe)
{
  char *pgpid = NULL;
  struct ABEntry *ab = NULL;

  if(AB_SearchEntry(pe->RealName, ASM_REALNAME|ASM_USER, &ab) == 0)
  {
    AB_SearchEntry(pe->Address, ASM_ADDRESS|ASM_USER, &ab);
  }

  if(!ab) return(NULL);

  if (ab->PGPId[0]) pgpid = ab->PGPId;

  return pgpid;
}
///
/// WR_GetPGPIds
//  Collects PGP key ids for all persons in a recipient field
static char *WR_GetPGPIds(char *source, char *ids)
{
   struct Person pe;
   char *next, *pid;

   for (; source; source = next)
   {
      if (!*source) break;
      if ((next = MyStrChr(source, ','))) *next++ = 0;
      ExtractAddress(source, &pe);
      if (!(pid = WR_GetPGPId(&pe)))
      {
         pid = pe.RealName[0] ? pe.RealName : pe.Address;
         ER_NewError(tr(MSG_ER_ErrorNoPGPId), source, pid);
      }
      ids = StrBufCat(ids, (G->PGPVersion == 5) ? "-r \"" : "\"");
      ids = StrBufCat(ids, pid);
      ids = StrBufCat(ids, "\" ");
   }
   return ids;
}
///
/// WR_Bounce
//  Bounce message: inserts resent-headers while copying the message
static BOOL WR_Bounce(FILE *fh, struct Compose *comp)
{
  FILE *oldfh;

  ENTER();

  if(comp->refMail != NULL && (oldfh = fopen(GetMailFile(NULL, NULL, comp->refMail), "r")))
  {
    char buf[SIZE_LINE];
    BOOL infield = FALSE;
    BOOL inbody = FALSE;

    setvbuf(oldfh, NULL, _IOFBF, SIZE_FILEBUF);

    while(fgets(buf, SIZE_LINE, oldfh))
    {
      if(*buf == '\n' && !inbody)
      {
        inbody = TRUE;
        EmitRcptHeader(fh, "To", comp->MailTo);
        EmitHeader(fh, "Resent-From", BuildAddrName(C->EmailAddress, C->RealName));
        EmitHeader(fh, "Resent-Date", GetDateTime());
      }

      if(!isspace(*buf) && !inbody)
        infield = !strnicmp(buf, "to:", 3);

      if(!infield || inbody)
        fputs(buf, fh);
    }
    fclose(oldfh);

    RETURN(TRUE);
    return TRUE;
  }

  RETURN(FALSE);
  return FALSE;
}

///
/// WR_SaveDec
//  Creates decrypted copy of a PGP encrypted message
static BOOL WR_SaveDec(FILE *fh, struct Compose *comp)
{
  char *mailfile;
  BOOL result = FALSE;

  ENTER();

  if(comp->refMail != NULL && (mailfile = GetMailFile(NULL, NULL, comp->refMail)))
  {
    char unpFile[SIZE_PATHFILE];
    BOOL xpkPacked = FALSE;
    FILE *oldfh;

    // we need to analyze if the folder we are reading this mail from
    // is encrypted or compressed and then first unpacking it to a temporary file
    if(isXPKFolder(comp->refMail->Folder))
    {
      // so, this mail seems to be packed, so we need to unpack it to a temporary file
      if(StartUnpack(mailfile, unpFile, comp->refMail->Folder) &&
         stricmp(mailfile, unpFile) != 0)
      {
        xpkPacked = TRUE;
      }
      else
      {
        RETURN(FALSE);
        return FALSE;
      }
    }

    if((oldfh = fopen(xpkPacked ? unpFile : mailfile, "r")))
    {
      BOOL infield = FALSE;
      char buf[SIZE_LINE];

      setvbuf(oldfh, NULL, _IOFBF, SIZE_FILEBUF);

      while(fgets(buf, SIZE_LINE, oldfh))
      {
        if(*buf == '\n')
        {
          fprintf(fh, "X-YAM-Decrypted: PGP; %s\n", GetDateTime());
          break;
        }

        if(!isspace(*buf))
        {
          infield = !strnicmp(buf, "content-type:", 13) ||
                    !strnicmp(buf, "content-transfer-encoding", 25) ||
                    !strnicmp(buf, "mime-version:", 13);
        }

        if(!infield)
          fputs(buf, fh);
      }

      fclose(oldfh);
      result = TRUE;
    }

    // if we temporary unpacked the file we delete it now
    if(xpkPacked)
      DeleteFile(unpFile);
  }

  RETURN(result);
  return result;
}

///
/// WR_EmitExtHeader
//  Outputs special X-YAM-Header lines to remember user-defined headers
static void WR_EmitExtHeader(FILE *fh, struct Compose *comp)
{
   if (*comp->ExtHeader)
   {
      char *p, ch = '\n';
      for (p = comp->ExtHeader; *p; ++p)
      {
         if (ch == '\n') fputs("X-YAM-Header-", fh);
         if (*p != '\\') ch = *p;
         else if (*++p == '\\') ch = '\\'; else if (*p == 'n') ch = '\n';
         fputc(ch, fh);
      }
      if (ch != '\n') fputc('\n', fh);
   }
}

///
/// WR_ComposeReport
//  Assembles the parts of a message disposition notification
static const char *MIMEwarn =
  "Warning: This is a message in MIME format. Your mail reader does\n"
  "not support MIME. Some parts of this message will be readable as\n"
  "plain text. To see the rest, you will need to upgrade your mail\n"
  "reader. Following are some URLs where you can find MIME-capable\n"
  "mail programs for common platforms:\n"
  "\n"
  "  AmigaOS...........: http://www.yam.ch/\n"
  "  Unix/MacOS/Windows: http://www.mozilla.com/thunderbird/\n"
  "\n"
  "General information about MIME can be found at:\n"
  "http://en.wikipedia.org/wiki/MIME\n";
static const char *PGPwarn  =
  "The following body part contains a PGP encrypted message. Either\n"
  "your mail reader doesn't support MIME/PGP as specified in RFC 2015,\n"
  "or the message was encrypted for someone else. To read the encrypted\n"
  "message, run the next body part through Pretty Good Privacy (PGP).\n\n";

static void WR_ComposeReport(FILE *fh, struct Compose *comp, char *boundary)
{
   struct WritePart *p;

   ENTER();

   fprintf(fh, "Content-type: multipart/report; report-type=disposition-notification; boundary=\"%s\"\n\n", boundary);

   for (p = comp->FirstPart; p; p = p->Next)
   {
      fprintf(fh, "\n--%s\n", boundary);
      WriteContentTypeAndEncoding(fh, p);
      fputs("\n", fh);
      EncodePart(fh, p);
   }
   fprintf(fh, "\n--%s--\n\n", boundary);

   LEAVE();
}

///
/// SetDefaultSecurity
static BOOL SetDefaultSecurity(struct Compose *comp)
{
  BOOL result = TRUE;
  enum Security security = SEC_NONE;
  BOOL FirstAddr = TRUE;
  char *CheckThese[3];
  unsigned int i;

  ENTER();

  // collect address pointers for easier iteration
  CheckThese[0] = comp->MailTo;
  CheckThese[1] = comp->MailCC;
  CheckThese[2] = comp->MailBCC;

  // go through all addresses
  for(i=0; i < ARRAY_SIZE(CheckThese); i++)
  {
    char *buf;

    // skip empty fields
    if(CheckThese[i] == NULL)
      continue;

    // copy string as strtok() will modify it
    if((buf = strdup(CheckThese[i])))
    {
      struct ABEntry *ab=NULL;
      enum Security currsec;
      char *in=buf;
      char *s;
      char *saveptr1;

      // loop through comma-separated addresses in string
      while((s = strtok_r(in, ",", &saveptr1)))
      {
        char *saveptr2;
        char *t = NULL;

        in = NULL;

        while((t = strtok_r(s, " ()<>", &saveptr2)))
        {
          s = NULL;

          if(strchr(t, '@'))
            break;
        }

        // can't find address for this entry - shouldn't happen
        if(!t)
          continue;

        if(AB_SearchEntry(t, ASM_ADDRESS|ASM_USER|ASM_COMPLETE, &ab) && (ab != NULL))
        {
          // get default from entry
          currsec = ab->DefSecurity;
        }
        else
          currsec = SEC_NONE; // entry not in address book -> no security

        if(currsec != security)
        {
          // first address' setting is always used
          if(FirstAddr)
          {
            FirstAddr = FALSE;
            security = currsec; // assume as default
          }
          else
          {
            // conflict: two addresses have different defaults
            int res = MUI_RequestA(G->App, NULL, 0, NULL, tr(MSG_WR_SECURITYREQ_GADS),
                                                          tr(MSG_WR_SECURITYREQ), NULL);

            switch(res)
            {
              case 0:
                result = FALSE;
              break;

              case 1:
                security = SEC_NONE;
              break;

              case 2:
                security = SEC_SIGN;
              break;

              case 3:
                security = SEC_ENCRYPT;
              break;

              case 4:
                security = SEC_BOTH;
              break;

              case 5:
                security = SEC_SENDANON;
              break;
            }

            break;  // terminate recipient loop
          }
        }
      }

      free(buf);
    }
  }

  comp->Security = security;

  RETURN(result);
  return result;
}

///
/// WR_ComposePGP
//  Creates a signed and/or encrypted PGP/MIME message
static BOOL WR_ComposePGP(FILE *fh, struct Compose *comp, char *boundary)
{
  enum Security sec = comp->Security;
  BOOL success = FALSE;
  struct WritePart pgppart, *firstpart = comp->FirstPart;
  char *ids = AllocStrBuf(SIZE_DEFAULT), pgpfile[SIZE_PATHFILE], options[SIZE_LARGE];
  struct TempFile *tf, *tf2;

  ENTER();

  pgppart.Filename = pgpfile; *pgpfile = 0;
  pgppart.EncType = ENC_NONE;
  if((sec == SEC_ENCRYPT) || (sec == SEC_BOTH))
  {
    if(comp->MailTo) ids = WR_GetPGPIds(comp->MailTo, ids);
    if(comp->MailCC) ids = WR_GetPGPIds(comp->MailCC, ids);
    if(comp->MailBCC) ids = WR_GetPGPIds(comp->MailBCC, ids);
    if(C->EncryptToSelf && *C->MyPGPID)
    {
      if(G->PGPVersion == 5)
        ids = StrBufCat(ids, "-r ");
      ids = StrBufCat(ids, C->MyPGPID);
    }
  }

  if((tf2 = OpenTempFile(NULL)) != NULL)
  {
    if((tf = OpenTempFile("w")) != NULL)
    {
       WriteContentTypeAndEncoding(tf->FP, firstpart);
       fputc('\n', tf->FP);
       EncodePart(tf->FP, firstpart);
       fclose(tf->FP);
       tf->FP = NULL;
       ConvertCRLF(tf->Filename, tf2->Filename, TRUE);
       CloseTempFile(tf);

       snprintf(pgpfile, sizeof(pgpfile), "%s.asc", tf2->Filename);

       if((sec == SEC_SIGN) || (sec == SEC_BOTH))
         PGPGetPassPhrase();

       switch(sec)
       {
         case SEC_SIGN:
         {
           fprintf(fh, "Content-type: multipart/signed; boundary=\"%s\"; micalg=pgp-md5; protocol=\"application/pgp-signature\"\n\n%s\n--%s\n", boundary, MIMEwarn, boundary);
           WriteContentTypeAndEncoding(fh, firstpart);
           fputc('\n', fh);
           EncodePart(fh, firstpart);
           fprintf(fh, "\n--%s\nContent-Type: application/pgp-signature\n\n", boundary);

           snprintf(options, sizeof(options), (G->PGPVersion == 5) ? "-ab %s +batchmode=1 +force" : "-sab %s +bat +f", tf2->Filename);
           if(*C->MyPGPID != '\0')
           {
             strlcat(options, " -u ", sizeof(options));
             strlcat(options, C->MyPGPID, sizeof(options));
           }

           if(!PGPCommand((G->PGPVersion == 5) ? "pgps" : "pgp", options, 0))
             success = TRUE;
         }
         break;

         case SEC_ENCRYPT:
         {
           fprintf(fh, "Content-type: multipart/encrypted; boundary=\"%s\"; protocol=\"application/pgp-encrypted\"\n\n%s\n--%s\n", boundary, MIMEwarn, boundary);
           fprintf(fh, "Content-Type: application/pgp-encrypted\n\nVersion: 1\n\n%s\n--%s\nContent-Type: application/octet-stream\n\n", PGPwarn, boundary);
           snprintf(options, sizeof(options), (G->PGPVersion == 5) ? "-a %s %s +batchmode=1 +force" : "-ea %s %s +bat +f", tf2->Filename, ids);
           if(!PGPCommand((G->PGPVersion == 5) ? "pgpe" : "pgp", options, 0))
             success = TRUE;
         }
         break;

         case SEC_BOTH:
         {
           fprintf(fh, "Content-type: multipart/encrypted; boundary=\"%s\"; protocol=\"application/pgp-encrypted\"\n\n%s\n--%s\n", boundary, MIMEwarn, boundary);
           fprintf(fh, "Content-Type: application/pgp-encrypted\n\nVersion: 1\n\n%s\n--%s\nContent-Type: application/octet-stream\n\n", PGPwarn, boundary);
           snprintf(options, sizeof(options), (G->PGPVersion == 5) ? "-a %s %s +batchmode=1 +force -s" : "-sea %s %s +bat +f", tf2->Filename, ids);

           if(*C->MyPGPID != '\0')
           {
             strlcat(options, " -u ", sizeof(options));
             strlcat(options, C->MyPGPID, sizeof(options));
           }

           if(!PGPCommand((G->PGPVersion == 5) ? "pgpe" : "pgp", options, 0))
             success = TRUE;
         }
         break;

         default:
           // nothing
         break;
      }
      if(success)
        EncodePart(fh, &pgppart);
    }
    CloseTempFile(tf2);
  }
  if(*pgpfile)
    remove(pgpfile);
  fprintf(fh, "\n--%s--\n\n", boundary);
  FreeStrBuf(ids);
  PGPClearPassPhrase(!success);

  RETURN(success);
  return success;
}

///
/// WR_ComposeMulti
//  Assembles a multipart message
static void WR_ComposeMulti(FILE *fh, struct Compose *comp, char *boundary)
{
  struct WritePart *p;

  ENTER();

  fprintf(fh, "Content-type: multipart/mixed; boundary=\"%s\"\n\n", boundary);
  fputs(MIMEwarn, fh);

  for(p = comp->FirstPart; p; p = p->Next)
  {
    fprintf(fh, "\n--%s\n", boundary);

    WriteContentTypeAndEncoding(fh, p);

    if(comp->Security == SEC_SENDANON)
      WR_Anonymize(fh, comp->MailTo);

    fputs("\n", fh);

    EncodePart(fh, p);
  }

  fprintf(fh, "\n--%s--\n\n", boundary);

  LEAVE();
}

///
/// WriteOutMessage() (rec)
//  Outputs header and body of a new message
BOOL WriteOutMessage(struct Compose *comp)
{
   BOOL success;
   struct TempFile *tf=NULL;
   FILE *fh = comp->FH;
   struct WritePart *firstpart = comp->FirstPart;
   char boundary[SIZE_DEFAULT], options[SIZE_DEFAULT], *rcptto;

   ENTER();

   if (comp->Mode == NEW_BOUNCE)
   {
     if(comp->DelSend)
       EmitHeader(fh, "X-YAM-Options", "delsent");

     success = WR_Bounce(fh, comp);

     RETURN(success);
     return success;
   }
   else if (comp->Mode == NEW_SAVEDEC)
   {
      if(!WR_SaveDec(fh, comp))
      {
        RETURN(FALSE);
        return FALSE;
      }
      else
        goto mimebody;
   }

   if(!firstpart)
   {
     RETURN(FALSE);
     return FALSE;
   }

   /* encrypted multipart message requested? */
   if (firstpart->Next && comp->Security > SEC_NONE  && comp->Security <= SEC_BOTH)
   {
      struct Compose tcomp;
      FILE *tfh;

      if((tf = OpenTempFile(NULL)) && (tfh = fopen(tf->Filename, "w")))
      {
         setvbuf(tfh, NULL, _IOFBF, SIZE_FILEBUF);

         memcpy(&tcomp,comp,sizeof(tcomp));   // clone struct Compose
         tcomp.FH = tfh;                      // set new filehandle
         tcomp.Security = SEC_NONE;           // temp msg gets attachments and no security

         /* clear a few other fields to avoid redundancies */
         tcomp.MailCC = NULL;
         tcomp.MailBCC = NULL;
         tcomp.ExtHeader = NULL;
         tcomp.Importance = 0;
         tcomp.DelSend = FALSE;
         tcomp.RequestMDN = FALSE;
         tcomp.UserInfo = FALSE;

         if(WriteOutMessage(&tcomp))    /* recurse! */
         {
            struct WritePart *tpart = comp->FirstPart; /* save parts list so we're able to recover from a calloc() error */

            /* replace with single new part */
            if((comp->FirstPart = (struct WritePart *)calloc(1,sizeof(struct WritePart))))
            {
               comp->FirstPart->EncType = tpart->EncType;          /* reuse encoding */
               FreePartsList(tpart);                               /* free old parts list */
               comp->FirstPart->ContentType = "message/rfc822";    /* the only part is an email message */
               comp->FirstPart->Filename = tf->Filename;           /* set filename to tempfile */
               comp->Signature = 0;                                /* only use sig in enclosed mail */
            }
            else
            {
               /* no errormsg here - the window probably won't open anyway... */
               DisplayBeep(NULL);
               comp->FirstPart = tpart;     /* just restore old parts list */
               comp->Security = 0;          /* switch off security */
               /* we'll most likely get more errors further down :( */
            }
         }
         else
         {
            ER_NewError(tr(MSG_ER_PGPMultipart));
            comp->Security = 0;
         }

         fclose(tfh);
      }
      else
      {
         ER_NewError(tr(MSG_ER_PGPMultipart));
         comp->Security = 0;
      }
   }
   *options = 0;
   if (comp->DelSend) strlcat(options, ",delsent", sizeof(options));
   if (comp->Security) snprintf(&options[strlen(options)], sizeof(options)-strlen(options), ",%s", SecCodes[comp->Security]);
   if (comp->Signature) snprintf(&options[strlen(options)], sizeof(options)-strlen(options), ",sigfile%d", comp->Signature-1);
   if (*options) EmitHeader(fh, "X-YAM-Options", &options[1]);
   EmitRcptHeader(fh, "From", comp->From ? comp->From : BuildAddrName(C->EmailAddress, C->RealName));
   if (comp->ReplyTo) EmitRcptHeader(fh, "Reply-To", comp->ReplyTo);
   if (comp->MailTo) EmitRcptHeader(fh, "To", comp->Security == 4 ? C->ReMailer : comp->MailTo);
   if (comp->MailCC) EmitRcptHeader(fh, "CC", comp->MailCC);
   if (comp->MailBCC) EmitRcptHeader(fh, "BCC", comp->MailBCC);
   EmitHeader(fh, "Date", GetDateTime());
   fprintf(fh, "Message-ID: <%s>\n", NewID(TRUE));
   if (comp->IRTMsgID) EmitHeader(fh, "In-Reply-To", comp->IRTMsgID);
   rcptto = comp->ReplyTo ? comp->ReplyTo : (comp->From ? comp->From : C->EmailAddress);
   if(comp->RequestMDN) EmitRcptHeader(fh, "Disposition-Notification-To", rcptto);
   if (comp->Importance) EmitHeader(fh, "Importance", comp->Importance == 1 ? "High" : "Low");
   fprintf(fh, "X-Mailer: %s\n", yamverxmailer);
   if (comp->UserInfo) WR_WriteUserInfo(fh, comp->From);
   if (*C->Organization) EmitHeader(fh, "Organization", C->Organization);
   if (*comp->Subject) EmitHeader(fh, "Subject", comp->Subject);
   if (comp->ExtHeader) WR_EmitExtHeader(fh, comp);

mimebody:

   fputs("MIME-Version: 1.0\n", fh); // RFC 1521 requires that

   strlcpy(boundary, NewID(FALSE), sizeof(boundary));

   if(comp->GenerateMDN)
   {
      WR_ComposeReport(fh, comp, boundary);
      success = TRUE;
   }
   else if(comp->Security > SEC_NONE && comp->Security <= SEC_BOTH)
   {
      success = WR_ComposePGP(fh, comp, boundary);
   }
   else if(firstpart->Next)
   {
      WR_ComposeMulti(fh, comp, boundary);
      success = TRUE;
   }
   else
   {
      WriteContentTypeAndEncoding(fh, firstpart);
      if (comp->Security == SEC_SENDANON && comp->OldSecurity != SEC_SENDANON)
         WR_Anonymize(fh, comp->MailTo);
      fputs("\n", fh);
      EncodePart(fh, firstpart);
      success = TRUE;
   }

   CloseTempFile(tf);

   RETURN(success);
   return success;
}

///
/// WR_AutoSaveFile
//  Returns filename of the auto-save file
char *WR_AutoSaveFile(int winnr)
{
  static char fname[SIZE_PATHFILE];
  strmfp(fname, G->MA_MailDir, ".autosave");
  strlcat(fname, itoa(winnr), sizeof(fname));
  strlcat(fname, ".txt", sizeof(fname));
  return fname;
}
///

/*** Buttons ***/
/// WR_NewMail
//  Validates write window options and generates a new message
void WR_NewMail(enum WriteMode mode, int winnum)
{
  char newMailFile[SIZE_PATHFILE];
  struct Compose comp;
  struct Mail *newMail = NULL;
  char *addr;


  struct Mail *mlist[3];
  int numAttachments = 0;
  struct WR_ClassData *wr = G->WR[winnum];
  struct WR_GUIData *gui = &wr->GUI;
  struct Folder *outfolder = FO_GetFolderByType(FT_OUTGOING, NULL);
  BOOL quietMode = (winnum == 2);
  BOOL winOpen = xget(gui->WI, MUIA_Window_Open);

  ENTER();

  // Workaround for a MUI bug
  if(winOpen)
    set(gui->RG_PAGE, MUIA_Group_ActivePage, xget(gui->RG_PAGE, MUIA_Group_ActivePage));

  // clear some variables we fill up later on
  memset(&comp, 0, sizeof(struct Compose));
  mlist[0] = (struct Mail *)1;
  mlist[1] = NULL;
  mlist[2] = NULL;

  // first we check all input values and fill up
  // the struct Compose variable

  // get the contents of the TO: String gadget and check if it is valid
  addr = (char *)DoMethod(gui->ST_TO, MUIM_Recipientstring_Resolve, MUIF_Recipientstring_Resolve_NoValid);
  if(!addr)
  {
    ER_NewError(tr(MSG_ER_AliasNotFound), (STRPTR)xget(gui->ST_TO, MUIA_String_Contents));

    if(winOpen)
      set(gui->RG_PAGE, MUIA_Group_ActivePage, 0);

    set(gui->WI, MUIA_Window_ActiveObject, gui->ST_TO);

    LEAVE();
    return;
  }
  else if(!addr[0] && quietMode == FALSE)
  {
    // set the TO Field active and go back
    if(winOpen)
      set(gui->RG_PAGE, MUIA_Group_ActivePage, 0);

    set(gui->WI, MUIA_Window_ActiveObject, gui->ST_TO);

    if(MUI_Request(G->App, gui->WI, 0, NULL, tr(MSG_WR_NoRcptReqGad), tr(MSG_WR_ErrorNoRcpt)))
      mode = WRITE_HOLD;
    else
    {
      LEAVE();
      return;
    }
  }
  else
    comp.MailTo = addr; // To: address

  // get the content of the Subject: String gadget and check if it is empty or not.
  comp.Subject = (char *)xget(gui->ST_SUBJECT, MUIA_String_Contents);
  if(wr->Mode != NEW_BOUNCE && quietMode == FALSE && C->WarnSubject &&
     (comp.Subject == NULL || comp.Subject[0] == '\0'))
  {
    if(winOpen)
      set(gui->RG_PAGE, MUIA_Group_ActivePage, 0);

    set(gui->WI, MUIA_Window_ActiveObject, gui->ST_SUBJECT);

    if(!MUI_Request(G->App, gui->WI, 0, NULL, tr(MSG_WR_OKAYCANCELREQ), tr(MSG_WR_NOSUBJECTREQ)))
    {
      LEAVE();
      return;
    }
  }

  comp.Mode = wr->Mode;
  comp.refMail = wr->refMail;
  comp.OldSecurity = wr->OldSecurity;

  if(wr->Mode != NEW_BOUNCE)
  {
    // now we check the From gadget and raise an error if is invalid
    addr = (char *)DoMethod(gui->ST_FROM, MUIM_Recipientstring_Resolve, MUIF_Recipientstring_Resolve_NoValid);
    if(!addr)
    {
      ER_NewError(tr(MSG_ER_AliasNotFound), (STRPTR)xget(gui->ST_FROM, MUIA_String_Contents));

      if(winOpen)
        set(gui->RG_PAGE, MUIA_Group_ActivePage, 2);

      set(gui->WI, MUIA_Window_ActiveObject, gui->ST_FROM);

      LEAVE();
      return;
    }
    else if(!addr[0] && quietMode == FALSE)
    {
      // set the TO Field active and go back
      if(winOpen)
        set(gui->RG_PAGE, MUIA_Group_ActivePage, 2);

      set(gui->WI, MUIA_Window_ActiveObject, gui->ST_FROM);

      if(!MUI_Request(G->App, gui->WI, 0, NULL, tr(MSG_WR_NOSENDERREQGAD), tr(MSG_WR_ERRORNOSENDER)))
      {
        LEAVE();
        return;
      }
    }
    else
      comp.From = addr;

    // then we check the CC string gadget
    addr = (char *)DoMethod(gui->ST_CC, MUIM_Recipientstring_Resolve, MUIF_Recipientstring_Resolve_NoValid);
    if(!addr)
    {
      ER_NewError(tr(MSG_ER_AliasNotFound), (STRPTR)xget(gui->ST_CC, MUIA_String_Contents));

      if(winOpen)
        set(gui->RG_PAGE, MUIA_Group_ActivePage, 2);

      set(gui->WI, MUIA_Window_ActiveObject, gui->ST_CC);

      LEAVE();
      return;
    }
    else if(addr[0] != '\0')
      comp.MailCC = addr;

    // then we check the BCC string gadget
    addr = (char *)DoMethod(gui->ST_BCC, MUIM_Recipientstring_Resolve, MUIF_Recipientstring_Resolve_NoValid);
    if(!addr)
    {
      ER_NewError(tr(MSG_ER_AliasNotFound), (STRPTR)xget(gui->ST_BCC, MUIA_String_Contents));

      if(winOpen)
        set(gui->RG_PAGE, MUIA_Group_ActivePage, 2);

      set(gui->WI, MUIA_Window_ActiveObject, gui->ST_BCC);

      LEAVE();
      return;
    }
    else if(addr[0] != '\0')
      comp.MailBCC = addr;

    // then we check the ReplyTo string gadget
    addr = (char *)DoMethod(gui->ST_REPLYTO, MUIM_Recipientstring_Resolve, MUIF_Recipientstring_Resolve_NoValid);
    if(!addr)
    {
      ER_NewError(tr(MSG_ER_AliasNotFound), (STRPTR)xget(gui->ST_REPLYTO, MUIA_String_Contents));

      if(winOpen)
        set(gui->RG_PAGE, MUIA_Group_ActivePage, 2);

      set(gui->WI, MUIA_Window_ActiveObject, gui->ST_REPLYTO);

      LEAVE();
      return;
    }
    else if(addr[0] != '\0')
      comp.ReplyTo = addr;

    comp.ExtHeader = (char *)xget(gui->ST_EXTHEADER, MUIA_String_Contents);

    if(wr->MsgID[0] != '\0')
      comp.IRTMsgID = wr->MsgID;

    comp.Importance = 1-GetMUICycle(gui->CY_IMPORTANCE);
    comp.RequestMDN = GetMUICheck(gui->CH_MDN);
    comp.Signature = GetMUIRadio(gui->RA_SIGNATURE);

    if((comp.Security = GetMUIRadio(gui->RA_SECURITY)) == SEC_DEFAULTS &&
       SetDefaultSecurity(&comp) == FALSE)
    {
      LEAVE();
      return;
    }

    comp.DelSend = GetMUICheck(gui->CH_DELSEND);
    comp.UserInfo = GetMUICheck(gui->CH_ADDINFO);

    numAttachments = xget(gui->LV_ATTACH, MUIA_NList_Entries);

    // we execute the POSTWRITE macro right before writing out
    // the message because the postwrite macro may want to modify the
    // text in the editor beforehand.
    MA_StartMacro(MACRO_POSTWRITE, itoa(winnum));

    // export the text of our texteditor to a file
    EditorToFile(gui->TE_EDIT, G->WR_Filename[winnum]);
    comp.FirstPart = BuildPartsList(winnum);
  }
  else
    MA_StartMacro(MACRO_POSTWRITE, itoa(winnum));

  // now we check how the new mail file should be named
  // or created off.
  switch(wr->Mode)
  {
    case NEW_EDIT:
    {
      if(wr->refMail != NULL && MailExists(wr->refMail, NULL))
      {
        GetMailFile(newMailFile, outfolder, wr->refMail);
        break;
      }
    }
    // continue

    case NEW_EDITASNEW:
      wr->Mode = NEW_NEW;
    // continue

    default:
      strlcpy(newMailFile, MA_NewMailFile(outfolder, NULL), sizeof(newMailFile));
  }

  // now open the new mail file for write operations
  if(newMailFile[0] != '\0' &&
     (comp.FH = fopen(newMailFile, "w")))
  {
    struct ExtendedMail *email;
    int stat = mode == WRITE_HOLD ? SFLAG_HOLD : SFLAG_QUEUED;

    setvbuf(comp.FH, NULL, _IOFBF, SIZE_FILEBUF);

    // write out the message to our file and
    // check that everything worked out fine.
    if(WriteOutMessage(&comp) == FALSE)
    {
      fclose(comp.FH);

      DeleteFile(newMailFile);

      LEAVE();
      return;
    }
    fclose(comp.FH);

    if(wr->Mode != NEW_BOUNCE)
      EndNotify(&G->WR_NRequest[winnum]);

    if((email = MA_ExamineMail(outfolder, FilePart(newMailFile), C->EmailCache > 0 ? TRUE : FALSE)))
    {
      email->Mail.sflags = stat;

      if((newMail = AddMailToList((struct Mail *)email, outfolder)) != NULL)
      {
        // Now we have to check whether we have to add the To & CC addresses
        // to the emailCache
        if(C->EmailCache > 0)
        {
          DoMethod(_app(gui->WI), MUIM_YAM_AddToEmailCache, &newMail->To);

          // if this mail has more than one recipient we have to add the others too
          if(isMultiRCPTMail(newMail))
          {
            int j;

            for(j = 0; j < email->NoSTo; j++)
              DoMethod(_app(gui->WI), MUIM_YAM_AddToEmailCache, &email->STo[j]);

            for(j = 0; j < email->NoCC; j++)
              DoMethod(_app(gui->WI), MUIM_YAM_AddToEmailCache, &email->CC[j]);

            for(j = 0; j < email->NoBCC; j++)
              DoMethod(_app(gui->WI), MUIM_YAM_AddToEmailCache, &email->BCC[j]);
          }
        }

        if(FO_GetCurrentFolder() == outfolder)
          DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_NList_InsertSingle, newMail, MUIV_NList_Insert_Sorted);

        MA_UpdateMailFile(newMail);

        // if this write operation was an edit mode
        // we have to check all existing readmail objects for
        // references and update them accordingly.
        if(wr->Mode == NEW_EDIT && wr->refMail != NULL)
        {
          struct MinNode *curNode;

          // now we search through our existing readMailData
          // objects and see some of them are pointing to the old mail
        // and if so we signal them to display the new revised mail instead
          for(curNode = G->readMailDataList.mlh_Head; curNode->mln_Succ; curNode = curNode->mln_Succ)
          {
            struct ReadMailData *rmData = (struct ReadMailData *)curNode;

            if(rmData->mail == wr->refMail)
            {
              if(rmData->readWindow)
                DoMethod(rmData->readWindow, MUIM_ReadWindow_ReadMail, newMail);
              else if(rmData->readMailGroup)
                DoMethod(rmData->readMailGroup, MUIM_ReadMailGroup_ReadMail, newMail);
            }
          }

          RemoveMailFromList(wr->refMail, TRUE);
          wr->refMail = newMail;
        }
      }

      // cleanup the email structure
      MA_FreeEMailStruct(email);
    }

    if(wr->Mode != NEW_NEW)
    {
      struct Mail **ml;

      if(wr->refMailList != NULL)
      {
        ml = wr->refMailList;
      }
      else if(wr->refMail)
      {
        ml = mlist;
        mlist[2] = wr->refMail;
      }
      else
        ml = NULL;

      if(ml)
      {
        int i;

        for(i=0; i < (int)ml[0]; i++)
        {
          struct Mail *m = ml[i+2];

          if(m != NULL && !isVirtualMail(m) && m->Folder != NULL &&
             !isOutgoingFolder(m->Folder) && !isSentFolder(m->Folder))
          {
            // process MDN notifications
            if(hasStatusNew(m) || !hasStatusRead(m))
              RE_ProcessMDN(MDN_MODE_DISPLAY, m, FALSE, winnum==2);

            switch(wr->Mode)
            {
              case NEW_REPLY:
              {
                setStatusToReplied(m);
                DisplayStatistics(m->Folder, FALSE);
              }
              break;

              case NEW_FORWARD:
              case NEW_BOUNCE:
              {
                setStatusToForwarded(m);
                DisplayStatistics(m->Folder, FALSE);
              }
              break;

              default:
                // nothing
              break;
            }
          }
        }
      }
    }

    // if requested we make sure we also
    // output a log entry about the write operation
    switch(wr->Mode)
    {
      case NEW_NEW:
      case NEW_EDITASNEW:
        AppendLog(10, tr(MSG_LOG_Creating), AddrName(newMail->To), newMail->Subject, numAttachments);
      break;

      case NEW_REPLY:
      {
        if(wr->refMailList && wr->refMailList[2])
          AppendLog(11, tr(MSG_LOG_Replying), AddrName(wr->refMailList[2]->From), wr->refMailList[2]->Subject);
        else
          AppendLog(11, tr(MSG_LOG_Replying), "<unknown>", "<unknown>");
      }
      break;

      case NEW_FORWARD:
      {
        if(wr->refMailList && wr->refMailList[2])
          AppendLog(12, tr(MSG_LOG_Forwarding), AddrName(wr->refMailList[2]->From), wr->refMailList[2]->Subject, AddrName(newMail->To));
        else
          AppendLog(12, tr(MSG_LOG_Forwarding), "<unknown>", "<unknown>", AddrName(newMail->To));
      }
      break;

      case NEW_BOUNCE:
      {
        if(wr->refMail)
          AppendLog(13, tr(MSG_LOG_Bouncing), AddrName(wr->refMail->From), wr->refMail->Subject, AddrName(newMail->To));
        else
          AppendLog(13, tr(MSG_LOG_Bouncing), "<unknown>", "<unknown>", AddrName(newMail->To));
      }
      break;

      case NEW_EDIT:
      {
        AppendLog(14, tr(MSG_LOG_Editing), AddrName(newMail->From), AddrName(newMail->To), newMail->Subject);
      }
      break;

      case NEW_SAVEDEC:
        // not used
      break;
    }
  }
  else
     ER_NewError(tr(MSG_ER_CreateMailError));

  FreePartsList(comp.FirstPart);

  // cleanup certain references
  wr->refMail = NULL;

  if(wr->refMailList)
  {
    free(wr->refMailList);
    wr->refMailList = NULL;
  }

  // now we make sure we immediately send out the mail.
  if(mode == WRITE_SEND && newMail && !G->TR)
  {
    set(gui->WI, MUIA_Window_Open, FALSE);
    mlist[2] = newMail;
    TR_ProcessSEND(mlist);
  }

  // delete a possible autosave file
  DeleteFile(WR_AutoSaveFile(winnum));

  DisposeModulePush(&G->WR[winnum]);
  DisplayStatistics(outfolder, TRUE);

  LEAVE();
}

HOOKPROTONHNO(WR_NewMailFunc, void, int *arg)
{
  BusyText(tr(MSG_BusyComposing), "");
  WR_NewMail(arg[0], arg[1]);
  BusyEnd();
}
MakeHook(WR_NewMailHook, WR_NewMailFunc);

///
/// WR_Cleanup
//  Terminates file notification and removes temporary files
void WR_Cleanup(int winnum)
{
  if(G->WR[winnum]->Mode != NEW_BOUNCE)
  {
    int i;

    EndNotify(&G->WR_NRequest[winnum]);
    DeleteFile(G->WR_Filename[winnum]);

    for(i=0; ;i++)
    {
      struct Attach *att;

      DoMethod(G->WR[winnum]->GUI.LV_ATTACH, MUIM_NList_GetEntry, i, &att);
      if(!att)
        break;

      if(att->IsTemp)
        DeleteFile(att->FilePath);
    }

    // delete a possible autosave file
    DeleteFile(WR_AutoSaveFile(winnum));
  }
}

///
/// WR_CancelFunc
/*** WR_CancelFunc - User clicked the Cancel button ***/
HOOKPROTONHNO(WR_CancelFunc, void, int *arg)
{
   int winnum = *arg;

   if (G->WR[winnum]->Mode != NEW_BOUNCE)
   {
      if (winnum < 2)
      {
         int haschanged = xget(G->WR[winnum]->GUI.TE_EDIT, MUIA_TextEditor_HasChanged);
         if (haschanged)
         {
            switch (MUI_Request(G->App, G->WR[winnum]->GUI.WI, 0, NULL, tr(MSG_WR_DiscardChangesGad), tr(MSG_WR_DiscardChanges)))
            {
               case 0: return;
               case 1: WR_NewMail(WRITE_QUEUE, winnum);
                       return;
               case 2: break;
            }
         }
      }
      WR_Cleanup(winnum);
   }
   DisposeModulePush(&G->WR[winnum]);
}
MakeStaticHook(WR_CancelHook, WR_CancelFunc);

///
/// WR_SaveAsFunc
/*** WR_SaveAsFunc - Saves contents of internal editor to a file ***/
HOOKPROTONHNO(WR_SaveAsFunc, void, int *arg)
{
  struct FileReqCache *frc;
  int winnum = *arg;

  ENTER();

  if(xget(G->WR[winnum]->GUI.WI, MUIA_Window_Open))
    set(G->WR[winnum]->GUI.RG_PAGE, MUIA_Group_ActivePage, 0);

  if((frc = ReqFile(ASL_ATTACH, G->WR[winnum]->GUI.WI, tr(MSG_WR_SaveTextAs), REQF_SAVEMODE, C->AttachDir, "")))
  {
    char filename[SIZE_PATHFILE];

    strmfp(filename, frc->drawer, frc->file);

    EditorToFile(G->WR[winnum]->GUI.TE_EDIT, G->WR_Filename[winnum]);

    if(!CopyFile(filename, NULL, G->WR_Filename[winnum], NULL))
      ER_NewError(tr(MSG_ER_CantCreateFile), filename);
  }

  LEAVE();
}
MakeStaticHook(WR_SaveAsHook, WR_SaveAsFunc);

///
/// WR_Edit
/*** WR_Edit - Launches external editor with message text ***/
HOOKPROTONHNO(WR_Edit, void, int *arg)
{
   if (*(C->Editor))
   {
      int winnum = *arg;
      char buffer[SIZE_COMMAND+SIZE_PATHFILE];

      /* Workaround for a MUI bug */
      if(xget(G->WR[winnum]->GUI.WI, MUIA_Window_Open))
        set(G->WR[winnum]->GUI.RG_PAGE, MUIA_Group_ActivePage, 0);

      EditorToFile(G->WR[winnum]->GUI.TE_EDIT, G->WR_Filename[winnum]);
      snprintf(buffer, sizeof(buffer), "%s \"%s\"", C->Editor, GetRealPath(G->WR_Filename[winnum]));
      ExecuteCommand(buffer, TRUE, OUT_NIL);
   }
}
MakeHook(WR_EditHook, WR_Edit);

///
/// WR_AddFileFunc
/*** WR_AddFileFunc - Adds one or more files to the attachment list ***/
HOOKPROTONHNO(WR_AddFileFunc, void, int *arg)
{
  int winnum = *arg;
  struct FileReqCache *frc;

  ENTER();

  if((frc = ReqFile(ASL_ATTACH, G->WR[winnum]->GUI.WI, tr(MSG_WR_AddFile), REQF_MULTISELECT, C->AttachDir, "")))
  {
    char filename[SIZE_PATHFILE];

    if(frc->numArgs == 0)
    {
      D(DBF_GUI, "choosen file: [%s] from drawer: [%s]", frc->file, frc->drawer);

      strmfp(filename, frc->drawer, frc->file);
      WR_AddFileToList(winnum, filename, NULL, FALSE);
    }
    else
    {
      int i;

      for(i=0; i < frc->numArgs; i++)
      {
        D(DBF_GUI, "choosen file: [%s] from drawer: [%s]", frc->argList[i], frc->drawer);

        strmfp(filename, frc->drawer, frc->argList[i]);
        WR_AddFileToList(winnum, filename, NULL, FALSE);
      }
    }
  }

  LEAVE();
}
MakeStaticHook(WR_AddFileHook, WR_AddFileFunc);

///
/// WR_AddArchiveFunc
/*** WR_AddArchiveFunc - Creates an archive of one or more files and adds it to the attachment list ***/
HOOKPROTONHNO(WR_AddArchiveFunc, void, int *arg)
{
  int winnum = *arg;
  BOOL result = FALSE;
  struct FileReqCache *frc;

  ENTER();

  // request files from the user via asl.library
  if((frc = ReqFile(ASL_ATTACH, G->WR[winnum]->GUI.WI, tr(MSG_WR_AddFile), REQF_MULTISELECT, C->AttachDir, "")) &&
      frc->numArgs > 0)
  {
    char *p;
    char arcname[SIZE_FILE];

    // get the basename of the first selected file and prepare it to the user
    // as a suggestion for an archive name
    strlcpy(arcname, FilePart(frc->argList[0]), sizeof(arcname));
    if((p = strrchr(arcname, '.')))
      *p = '\0';

    // now ask the user for the choosen archive name
    if(StringRequest(arcname, SIZE_FILE, tr(MSG_WR_CreateArc), tr(MSG_WR_CreateArcReq), tr(MSG_Okay), NULL, tr(MSG_Cancel), FALSE, G->WR[winnum]->GUI.WI))
    {
      char *command;
      char filename[SIZE_PATHFILE];
      char arcpath[SIZE_PATHFILE];
      struct TempFile *tf = NULL;

      // create the destination archive name
      strmfp(filename, C->TempDir, arcname);
      snprintf(arcpath, sizeof(arcpath), strchr(filename, ' ') ? "\"%s\"" : "%s", filename);

      // now we generate the temporary file containing the file names
      // which we are going to put in the archive.
      if(strstr(C->PackerCommand, "%l") && (tf = OpenTempFile("w")))
      {
        int i;

        for(i=0; i < frc->numArgs; i++)
          fprintf(tf->FP, strchr(frc->argList[i], ' ') ? "\"%s\"\n" : "%s\n", frc->argList[i]);

        // just close here as we delete it later on
        fclose(tf->FP);
        tf->FP = NULL;
      }

      // now we create the command we are going to
      // execute for generating the archive.
      if((command = AllocStrBuf(SIZE_DEFAULT)))
      {
        char *src;
        BPTR filedir;

        for(src = C->PackerCommand; *src; src++)
        {
          if(*src == '%')
          {
            switch (*(++src))
            {
              case '%': command = StrBufCat(command, "%"); break;
              case 'a': command = StrBufCat(command, arcpath); break;
              case 'l': command = StrBufCat(command, tf->Filename); break;

              case 'f':
              {
                int i;

                for(i=0; i < frc->numArgs; i++)
                {
                  snprintf(filename, sizeof(filename), "\"%s\" ", frc->argList[i]);
                  command = StrBufCat(command, filename);
                }
                break;
              }
              break;
            }
          }
          else
          {
            char chr[2];
            chr[0] = *src;
            chr[1] = '\0';

            command = StrBufCat(command, chr);
          }
        }

        // now we make the request drawer the current one temporarly.
        if((filedir = Lock(frc->drawer, ACCESS_READ)))
        {
          BPTR olddir = CurrentDir(filedir);

          // now we do the archive generation right here.
          result = ExecuteCommand(command, FALSE, OUT_NIL);

          // make the old directory the new current one again
          CurrentDir(olddir);
          UnLock(filedir);
        }

        // free our private resources
        FreeStrBuf(command);
        CloseTempFile(tf);

        // if everything worked out fine we go
        // and find out the real final attachment name
        if(result)
        {
          strlcpy(filename, arcpath, sizeof(filename));
          if(FileSize(filename) == -1)
          {
            snprintf(filename, sizeof(filename), "%s.lha", arcpath);
            if(FileSize(filename) == -1)
            {
              snprintf(filename, sizeof(filename), "%s.lzx", arcpath);
              if(FileSize(filename) == -1)
                snprintf(filename, sizeof(filename), "%s.zip", arcpath);
            }
          }

          WR_AddFileToList(winnum, filename, NULL, TRUE);
        }
        else
          ER_NewError(tr(MSG_ER_PACKERROR));
      }
    }
  }

  LEAVE();
}
MakeStaticHook(WR_AddArchiveHook, WR_AddArchiveFunc);

///
/// WR_DisplayFile
/*** WR_DisplayFile - Displays an attached file using a MIME viewer ***/
HOOKPROTONHNO(WR_DisplayFile, void, int *arg)
{
   struct Attach *attach = NULL;

   DoMethod(G->WR[*arg]->GUI.LV_ATTACH, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &attach);
   if (attach) RE_DisplayMIME(attach->FilePath, attach->ContentType);
}
MakeStaticHook(WR_DisplayFileHook, WR_DisplayFile);

///
/// WR_DeleteFile
// Deletes a file from the attachment list and the belonging temporary file
HOOKPROTONHNO(WR_DeleteFile, void, int *arg)
{
   struct Attach *attach = NULL;

   // first we get the active entry
   DoMethod(G->WR[*arg]->GUI.LV_ATTACH, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &attach);
   if(!attach) return;

   // then we remove the active entry from the NList
   DoMethod(G->WR[*arg]->GUI.LV_ATTACH, MUIM_NList_Remove, MUIV_NList_Remove_Active);

   // delete the temporary file if exists
   if(attach->IsTemp) DeleteFile(attach->FilePath);
}
MakeStaticHook(WR_DeleteFileHook, WR_DeleteFile);

///
/// WR_ChangeSignatureFunc
/*** WR_ChangeSignatureFunc - Changes the current signature ***/
HOOKPROTONHNO(WR_ChangeSignatureFunc, void, int *arg)
{
   struct TempFile *tf;
   int signat = arg[0], winnum = arg[1];
   char buffer[SIZE_LINE];
   FILE *in, *out;

   if ((tf = OpenTempFile(NULL)))
   {
      EditorToFile(G->WR[winnum]->GUI.TE_EDIT, tf->Filename);
      if ((in = fopen(tf->Filename, "r")))
      {
         setvbuf(in, NULL, _IOFBF, SIZE_FILEBUF);

         if ((out = fopen(G->WR_Filename[winnum], "w")))
         {
            setvbuf(out, NULL, _IOFBF, SIZE_FILEBUF);

            while(fgets(buffer, SIZE_LINE, in))
            {
              if(strcmp(buffer, "-- \n"))
                fputs(buffer, out);
              else
                break;
            }

            if(signat)
              WR_WriteSignature(out, signat-1);

            fclose(out);
         }

         fclose(in);
      }
      CloseTempFile(tf);;
   }
}
MakeStaticHook(WR_ChangeSignatureHook, WR_ChangeSignatureFunc);
///

/*** Menus ***/
/// WR_TransformText
//  Inserts or pastes text as plain, ROT13, uuencoded or quoted text
static char *WR_TransformText(const char *source, const enum TransformMode mode, const char *qtext)
{
  char *dest = NULL;
  int size;

  ENTER();

  if((size = FileSize(source)) > 0)
  {
    int qtextlen = strlen(qtext);
    BOOL quote = (mode == ED_INSQUOT || mode == ED_PASQUOT ||
                  mode == ED_INSALTQUOT || mode == ED_PASALTQUOT);

    size += SIZE_DEFAULT;

    if(quote)
      size += size/20*qtextlen;

    if(mode == ED_INSUUCODE)
      size += strlen(FilePart(qtext))+12+7; // for the "begin 644 XXX" and "end passage

    if((dest = calloc(size, 1)))
    {
      FILE *fp;

      if((fp = fopen(source, "r")))
      {
        int ch;
        int p=0;
        int pos=0;

        setvbuf(fp, NULL, _IOFBF, SIZE_FILEBUF);

        // in case the source is UUencoded text we have to add the
        // "begin 644 XXX" stuff in advance.
        if(mode == ED_INSUUCODE)
          p = snprintf(dest, size, "\nbegin 644 %s\n", FilePart(qtext));

        while((ch = fgetc(fp)) != EOF)
        {
          if(!pos && quote)
          {
            int i;

            if(p+qtextlen > size-2)
            {
              size += SIZE_LARGE;
              dest = realloc(dest, size);
              if(dest == NULL)
                break;
            }

            for(i=0; i < qtextlen; i++)
              dest[p++] = qtext[i];

            dest[p++] = ' ';
          }

          if(ch == '\n')
            pos = 0;
          else
            ++pos;

          if(mode == ED_INSROT13 || mode == ED_PASROT13)
          {
            if(ch >= 'a' && ch <= 'z' && ((ch += 13) > 'z'))
              ch -= 26;

            if(ch >= 'A' && ch <= 'Z' && ((ch += 13) > 'Z'))
              ch -= 26;
          }

          if(p > size-3)
          {
            size += SIZE_LARGE;
            dest = realloc(dest, size);
            if(dest == NULL)
              break;
          }

          dest[p++] = ch;
        }

        // a realloc call might have failed, so better check this
        if(dest != NULL)
          dest[p] = '\0';

        // in case the source is UUencoded text we have to add the
        // "end" stuff at the end of the text as well
        if(mode == ED_INSUUCODE)
          strlcat(dest, "``\nend\n", size);

        fclose(fp);
      }
    }
  }

  RETURN(dest);
  return dest;
}

///
/// WR_EditorCmd
/*** WR_EditorCmd - Inserts file or clipboard into editor ***/
HOOKPROTONHNO(WR_EditorCmd, void, int *arg)
{
  enum TransformMode cmd = arg[0];
  int winnum = arg[1];
  char *text = NULL;
  char *quoteChar;
  char filename[SIZE_PATHFILE];
  struct TempFile *tf;
  struct WR_ClassData *wr = G->WR[winnum];

  ENTER();

  quoteChar = (cmd == ED_INSALTQUOT || cmd == ED_PASALTQUOT) ? C->AltQuoteChar : C->QuoteChar;

  switch(cmd)
  {
    case ED_INSERT:
    case ED_INSQUOT:
    case ED_INSALTQUOT:
    case ED_INSROT13:
    case ED_OPEN:
    {
      struct FileReqCache *frc;

      if((frc = ReqFile(ASL_ATTACH, wr->GUI.WI, tr(MSG_WR_InsertFile), REQF_NONE, C->AttachDir, "")))
      {
        strmfp(filename, frc->drawer, frc->file);
        text = WR_TransformText(filename, cmd, quoteChar);
      }
      else
      {
        LEAVE();
        return;
      }
    }
    break;

    // the user wants to insert a file as
    // a uuencoded text passage.
    case ED_INSUUCODE:
    {
      struct FileReqCache *frc;

      // first we request the filename of the file we want to its
      // uuencoded interpretation to be inserted into the editor
      if((frc = ReqFile(ASL_ATTACH, wr->GUI.WI, tr(MSG_WR_InsertFile), REQF_NONE, C->AttachDir, "")))
      {
        strmfp(filename, frc->drawer, frc->file);

        // open a temporary file
        if((tf = OpenTempFile("w")))
        {
          // open both files and start the uuencoding
          FILE *in = fopen(filename, "r");
          if(in)
          {
            setvbuf(in, NULL, _IOFBF, SIZE_FILEBUF);

            // lets uuencode the file now.
            if(uuencode_file(in, tf->FP) > 0)
            {
              // we successfully encoded the file, so lets
              // close our tempfile file pointer and call the tranformtext function
              fclose(tf->FP);
              tf->FP = NULL;

              // now transform the text
              text = WR_TransformText(tf->Filename, cmd, filename);
            }

            fclose(in);
          }

          CloseTempFile(tf);
        }
      }
    }
    break;

    default:
    {
      if((tf = OpenTempFile("w")))
      {
        DumpClipboard(tf->FP);
        fclose(tf->FP);
        tf->FP = NULL;
        text = WR_TransformText(tf->Filename, cmd, quoteChar);
        CloseTempFile(tf);
      }
      else
      {
        LEAVE();
        return;
      }
    }
    break;
  }

  if(text)
  {
    if(cmd == ED_OPEN)
      DoMethod(wr->GUI.TE_EDIT, MUIM_TextEditor_ClearText);

    DoMethod(wr->GUI.TE_EDIT, MUIM_TextEditor_InsertText, text);

    // free our allocated text
    free(text);
  }

  LEAVE();
}
MakeHook(WR_EditorCmdHook, WR_EditorCmd);

///
/// WR_SearchFunc
/*** WR_SearchFunc - Opens a search window ***/
HOOKPROTONHNO(WR_SearchFunc, void, int *arg)
{
  Object *texteditor = (Object *)arg[0];

  ENTER();

  if(G->WI_SEARCH == NULL)
  {
    if((G->WI_SEARCH = SearchwindowObject, End))
    {
      DoMethod(G->App, OM_ADDMEMBER, G->WI_SEARCH);

      // perform the search operation
      DoMethod(G->WI_SEARCH, MUIM_Searchwindow_Open, texteditor);
    }
  }
  else
  {
    if(hasSearchAgainFlag(arg[1]))
      DoMethod(G->WI_SEARCH, MUIM_Searchwindow_Next);
    else
      DoMethod(G->WI_SEARCH, MUIM_Searchwindow_Open, texteditor);
  }

  LEAVE();
}
MakeHook(WR_SearchHook, WR_SearchFunc);

///
/// WR_AddClipboardFunc
/*** WR_AddClipboardFunc - Adds contents of clipboard as attachment ***/
HOOKPROTONHNO(WR_AddClipboardFunc, void, int *arg)
{
   int winnum = *arg;
   struct TempFile *tf = OpenTempFile("w");
   if (DumpClipboard(tf->FP))
   {
      fclose(tf->FP); tf->FP = NULL;
      WR_AddFileToList(winnum, tf->Filename, "clipboard.text", TRUE);
      free(tf);
      return;
   }
   CloseTempFile(tf);
}
MakeStaticHook(WR_AddClipboardHook, WR_AddClipboardFunc);

///
/// WR_AddPGPKeyFunc
/*** WR_AddPGPKeyFunc - Adds ASCII version of user's public PGP key as attachment ***/
HOOKPROTONHNO(WR_AddPGPKeyFunc, void, int *arg)
{
  int winnum = *arg;
  char *myid = *C->MyPGPID ? C->MyPGPID : C->EmailAddress;
  char options[SIZE_LARGE];
  const char  *fname = "T:PubKey.asc";

  snprintf(options, sizeof(options), (G->PGPVersion == 5) ? "-x %s -o %s +force +batchmode=1" : "-kxa %s %s +f +bat", myid, fname);

  if(!PGPCommand((G->PGPVersion == 5) ? "pgpk" : "pgp", options, 0))
  {
    if(FileSize(fname) > 0)
    {
      WR_AddFileToList(winnum, fname, NULL, TRUE);
      setstring(G->WR[winnum]->GUI.ST_CTYPE, "application/pgp-keys");
    }
    else
      ER_NewError(tr(MSG_ER_ErrorAppendKey), myid);
  }
}
MakeStaticHook(WR_AddPGPKeyHook, WR_AddPGPKeyFunc);
///

/*** Open ***/
/// WR_Open
//  Initializes a write window, returns -1 on error
int WR_Open(int winnum, BOOL bounce)
{
  int result = -1;

  // winnum:
  // -1 - a new window should be opened
  //  2 - window should be opened in "quiet mode"
  //
  // G->WR[0]: 1st possible window.
  // G->WR[1]: 2nd possible window.
  // G->WR[2]: Quiet window (last).

  ENTER();

  // Find first un-used window and set winnum accordingly (or return -1
  // if they are all taken).
  if(winnum != -1 ||
     G->WR[winnum = 0] == NULL ||
     G->WR[winnum = 1] == NULL)
  {
    if(G->WR[winnum] == NULL)
    {
      // open a new window
      G->WR[winnum] = bounce ? WR_NewBounce(winnum) : WR_New(winnum);
      if(G->WR[winnum] != NULL)
      {
        if(!bounce)
        {
          struct WR_GUIData *gui = &G->WR[winnum]->GUI;

          setstring(gui->ST_FROM, BuildAddrName(C->EmailAddress, C->RealName));
          setstring(gui->ST_REPLYTO, C->ReplyTo);
          setstring(gui->ST_EXTHEADER, C->ExtraHeaders);
          setcheckmark(gui->CH_DELSEND, !C->SaveSent);
          setcheckmark(gui->CH_ADDINFO, C->AddMyInfo);
          setcheckmark(gui->CH_MDN, C->RequestMDN);
        }

        MA_StartMacro(MACRO_PREWRITE, itoa(winnum));

        result = winnum;
      }
    }
    else
      E(DBF_MAIL, "new write window generated in slot %d, but old one still allocated?", winnum);
  }

  if(result == -1)
    DisplayBeep(NULL);

  RETURN(result);
  return result;
}

///
/// WR_SetupOldMail
/*** WR_SetupOldMail - When editing a message, sets write window options to old values ***/
void WR_SetupOldMail(int winnum, struct ReadMailData *rmData)
{
  static struct Attach attach;
  struct Part *part;

  // we start to iterate right from the first part *after* PART_RAW
  // and check which one really is really an attachment or which
  // on is the LetterPart
  for(part = rmData->firstPart->Next; part; part = part->Next)
  {
    if(part->Nr != rmData->letterPartNum &&
       stricmp(part->ContentType, "application/pgp-signature"))
    {
      BusyText(tr(MSG_BusyDecSaving), "");

      RE_DecodePart(part);
      memset(&attach, 0, sizeof(struct Attach));
      attach.Size = part->Size;
      attach.IsMIME = part->EncodingCode != ENC_UUE;
      attach.IsTemp = TRUE;

      if(part->Name)
        strlcpy(attach.Name, part->Name, sizeof(attach.Name));

      strlcpy(attach.FilePath, part->Filename, sizeof(attach.FilePath));
      *part->Filename = '\0';
      strlcpy(attach.ContentType, part->ContentType, sizeof(attach.ContentType));
      strlcpy(attach.Description, part->Description, sizeof(attach.Description));
      DoMethod(G->WR[winnum]->GUI.LV_ATTACH, MUIM_NList_InsertSingle, &attach, MUIV_NList_Insert_Bottom);

      BusyEnd();
    }
  }
}

///
/// WR_UpdateTitleFunc
/*** WR_UpdateTitleFunc - Shows cursor coordinates ***/
HOOKPROTONHNO(WR_UpdateWTitleFunc, void, int *arg)
{
  struct WR_ClassData *wr = G->WR[*arg];
  Object *ed = wr->GUI.TE_EDIT;

  snprintf(wr->WTitle, sizeof(wr->WTitle), "%03ld\n%03ld", xget(ed,MUIA_TextEditor_CursorY)+1, xget(ed,MUIA_TextEditor_CursorX)+1);
  set(wr->GUI.TX_POSI, MUIA_Text_Contents, wr->WTitle);
}
MakeStaticHook(WR_UpdateWTitleHook,WR_UpdateWTitleFunc);
///

/*** Hooks ***/
/// WR_App
/*** WR_App - Handles Drag&Drop ***/
void WR_App(int winnum, STRPTR fileName)
{
  int mode = xget(G->WR[winnum]->GUI.RG_PAGE, MUIA_Group_ActivePage);

  if(!mode)
  {
    FILE *fh;

    if((fh = fopen(fileName, "r")))
    {
      char buffer[SIZE_LARGE];
      int j;
      int notascii = 0;
      int len = fread(buffer, 1, SIZE_LARGE-1, fh);

      buffer[len] = '\0';
      fclose(fh);

      for(j = 0; j < len; j++)
      {
        int c = (int)buffer[j];

        if((c < 32 || c > 127) &&
            c != '\t' && c != '\n')
        {
          notascii++;
        }
      }

      if(notascii && len/notascii <= 16)
        mode = 1;
    }
  }

  if(!mode)
  {
    char *text;

    if((text = WR_TransformText(fileName, ED_INSERT, "")) != NULL)
    {
      DoMethod(G->WR[winnum]->GUI.TE_EDIT, MUIM_TextEditor_InsertText, text);
      free(text);
    }
    else
      WR_AddFileToList(winnum, fileName, NULL, FALSE);
  }
  else
    WR_AddFileToList(winnum, fileName, NULL, FALSE);
}

///
/// WR_AppFunc
HOOKPROTONHNO(WR_AppFunc, LONG, ULONG *arg)
{
  // manage the AppMessage
  struct AppMessage *amsg = (struct AppMessage *)arg[0];
  int winnum = (int)arg[1];
  int i;

  // lets walk through all arguments in the appMessage
  for(i = 0; i < amsg->am_NumArgs; i++)
  {
    char buf[SIZE_PATHFILE];
    struct WBArg *ap = &amsg->am_ArgList[i];

    NameFromLock(ap->wa_Lock, buf, sizeof(buf));
    AddPart(buf, (char *)ap->wa_Name, sizeof(buf));

    // call WR_App to let it put in the text of the file
    // to the write window
    WR_App(winnum, buf);
  }

  return 0;
}
MakeStaticHook(WR_AppHook, WR_AppFunc);

///
/// WR_LV_ConFunc
/*** WR_LV_ConFunc - Attachment listview construct hook ***/
HOOKPROTONHNO(WR_LV_ConFunc, struct Attach *, struct Attach *attach)
{
   struct Attach *entry = malloc(sizeof(struct Attach));

   if (entry)
     *entry = *attach;
   return entry;
}
MakeStaticHook(WR_LV_ConFuncHook, WR_LV_ConFunc);

///
/// WR_LV_DspFunc
/*** WR_LV_DspFunc - Attachment listview display hook ***/
HOOKPROTONH(WR_LV_DspFunc, long, char **array, struct Attach *entry)
{
   static char dispsz[SIZE_SMALL];
   if (entry)
   {
      array[0] = entry->Name;
      snprintf(array[1] = dispsz, sizeof(dispsz), "%d", entry->Size);
      array[2] = (STRPTR)DescribeCT(entry->ContentType);
      array[3] = (STRPTR)(entry->IsMIME ? "MIME" : "UU");
      array[4] = entry->Description;
   }
   else
   {
      array[0] = (STRPTR)tr(MSG_WR_TitleFile);
      array[1] = (STRPTR)tr(MSG_WR_TitleSize);
      array[2] = (STRPTR)tr(MSG_WR_TitleContents);
      array[3] = (STRPTR)tr(MSG_WR_TitleEncoding);
      array[4] = (STRPTR)tr(MSG_WR_TitleDescription);
   }
   return 0;
}
MakeStaticHook(WR_LV_DspFuncHook, WR_LV_DspFunc);
///
/// WR_SetSoftStyleFunc
HOOKPROTONHNO(WR_SetSoftStyleFunc, void, ULONG *arg)
{
  enum SoftStyleMode ssm = (enum SoftStyleMode)arg[0];
  int winnum = (int)arg[1];
  Object *textEdit = G->WR[winnum]->GUI.TE_EDIT;
  ULONG x1=0;
  ULONG y1=0;
  ULONG x2=0;
  ULONG y2=0;
  char *txt;

  ENTER();

  // check the marked text status
  if(DoMethod(textEdit, MUIM_TextEditor_BlockInfo, &x1, &y1, &x2, &y2) == FALSE)
  {
    x1 = xget(textEdit, MUIA_TextEditor_CursorX);
    y1 = xget(textEdit, MUIA_TextEditor_CursorY);
  }

  // retrieve the whole text lines where the cursor/block is currently
  // active at.
  if((txt = (char *)DoMethod(textEdit, MUIM_TextEditor_ExportBlock, MUIF_TextEditor_ExportBlock_FullLines)))
  {
    int txtlen = strlen(txt);

    if(txtlen > 0)
    {
      LONG nx1=x1;
      LONG nx2=x1;
      char *ntxt;

      // now we walk back from the starting (x1) position searching for the next space
      // or start of line
      while(nx1 > 0 && !isspace(txt[nx1]))
        nx1--;

      if(nx1 > 0 && (ULONG)nx1 != x1)
        nx1++;

      while(nx2 < txtlen && !isspace(txt[nx2]))
        nx2++;

      if(nx2-nx1 > 0)
      {
        char marker[2] = " ";
        BOOL disableStyle = FALSE;

        // define the marker for the
        // selected soft-style
        switch(ssm)
        {
          case SSM_NORMAL:
            // nothing;
          break;

          case SSM_BOLD:
            marker[0] = '*';
          break;

          case SSM_ITALIC:
            marker[0] = '/';
          break;

          case SSM_UNDERLINE:
            marker[0] = '_';
          break;

          case SSM_COLOR:
            marker[0] = '#';
          break;
        }

        // check if there is already some soft-style active
        // and if yes, we go and strip
        if(ssm != SSM_NORMAL &&
           (txt[nx1] == marker[0] && txt[nx2-1] == marker[0]))
        {
          if((ntxt = calloc(1, nx2-2-nx1+1+1)))
            strlcpy(ntxt, &txt[nx1+1], nx2-2-nx1+1);

          disableStyle = TRUE;
        }
        else
        {
          if((ntxt = calloc(1, nx2-nx1+2+1)))
          {
            ntxt[0] = marker[0];

            // strip foreign soft-style markers
            if((txt[nx1] == '*' && txt[nx2-1] == '*') ||
               (txt[nx1] == '/' && txt[nx2-1] == '/') ||
               (txt[nx1] == '_' && txt[nx2-1] == '_') ||
               (txt[nx1] == '#' && txt[nx2-1] == '#'))
            {
              strlcat(ntxt, &txt[nx1+1], nx2-nx1);
            }
            else
              strlcat(ntxt, &txt[nx1], nx2-nx1+2);

            strlcat(ntxt, marker, nx2-nx1+2+1);
          }
        }

        if(ntxt)
        {
          // now we mark the area we found to be the one we want to
          // set bold
          DoMethod(textEdit, MUIM_TextEditor_MarkText, nx1, y1, nx2, y1);

          // now we replace the marked area with the text we extract but adding two
          // bold signs at the back and setting the area bold
          DoMethod(textEdit, MUIM_TextEditor_Replace, ntxt, MUIF_NONE);

          // set the soft-style accordingly.
          if(disableStyle == FALSE)
          {
            DoMethod(textEdit, MUIM_TextEditor_MarkText, nx1, y1, nx1+strlen(ntxt), y1);

            // make sure the text in question is really in the style we want
            switch(ssm)
            {
              case SSM_NORMAL:
              {
                SetAttrs(textEdit, MUIA_TextEditor_StyleBold,      FALSE,
                                   MUIA_TextEditor_StyleItalic,    FALSE,
                                   MUIA_TextEditor_StyleUnderline, FALSE,
                                   MUIA_TextEditor_Pen,            0,
                                   TAG_DONE);
              }
              break;

              case SSM_BOLD:
              {
                SetAttrs(textEdit, MUIA_TextEditor_StyleBold,      TRUE,
                                   MUIA_TextEditor_StyleItalic,    FALSE,
                                   MUIA_TextEditor_StyleUnderline, FALSE,
                                   MUIA_TextEditor_Pen,            0,
                                   TAG_DONE);
              }
              break;

              case SSM_ITALIC:
              {
                SetAttrs(textEdit, MUIA_TextEditor_StyleBold,      FALSE,
                                   MUIA_TextEditor_StyleItalic,    TRUE,
                                   MUIA_TextEditor_StyleUnderline, FALSE,
                                   MUIA_TextEditor_Pen,            0,
                                   TAG_DONE);
              }
              break;

              case SSM_UNDERLINE:
              {
                SetAttrs(textEdit, MUIA_TextEditor_StyleBold,      FALSE,
                                   MUIA_TextEditor_StyleItalic,    FALSE,
                                   MUIA_TextEditor_StyleUnderline, TRUE,
                                   MUIA_TextEditor_Pen,            0,
                                   TAG_DONE);
              }
              break;

              case SSM_COLOR:
              {
                SetAttrs(textEdit, MUIA_TextEditor_StyleBold,      FALSE,
                                   MUIA_TextEditor_StyleItalic,    FALSE,
                                   MUIA_TextEditor_StyleUnderline, FALSE,
                                   MUIA_TextEditor_Pen,            7,
                                   TAG_DONE);
              }
              break;
            }
          }
          else
            DoMethod(textEdit, MUIM_TextEditor_MarkText, nx1, y1, nx2-2, y1);

          free(ntxt);
        }
      }
    }

    FreeVec(txt);
  }

  LEAVE();
}
MakeHook(WR_SetSoftStyleHook, WR_SetSoftStyleFunc);

///

/*** GUI ***/
/// WR_SharedSetup
//  Common setup for write and bounce windows
static void WR_SharedSetup(struct WR_ClassData *data, int winnum)
{
   SetHelp(data->GUI.ST_TO      ,MSG_HELP_WR_ST_TO      );
   SetHelp(data->GUI.BT_QUEUE   ,MSG_HELP_WR_BT_QUEUE   );
   SetHelp(data->GUI.BT_HOLD    ,MSG_HELP_WR_BT_HOLD    );
   SetHelp(data->GUI.BT_SEND    ,MSG_HELP_WR_BT_SEND    );
   SetHelp(data->GUI.BT_CANCEL  ,MSG_HELP_WR_BT_CANCEL  );
   DoMethod(data->GUI.BT_HOLD    ,MUIM_Notify,MUIA_Pressed             ,FALSE         ,MUIV_Notify_Application,4,MUIM_CallHook   ,&WR_NewMailHook,WRITE_HOLD,winnum);
   DoMethod(data->GUI.BT_QUEUE   ,MUIM_Notify,MUIA_Pressed             ,FALSE         ,MUIV_Notify_Application,4,MUIM_CallHook   ,&WR_NewMailHook,WRITE_QUEUE,winnum);
   DoMethod(data->GUI.BT_SEND    ,MUIM_Notify,MUIA_Pressed             ,FALSE         ,MUIV_Notify_Application,4,MUIM_CallHook   ,&WR_NewMailHook,WRITE_SEND,winnum);
   DoMethod(data->GUI.BT_CANCEL  ,MUIM_Notify,MUIA_Pressed             ,FALSE         ,MUIV_Notify_Application,3,MUIM_CallHook   ,&WR_CancelHook,winnum);
   DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_CloseRequest ,TRUE          ,MUIV_Notify_Application,3,MUIM_CallHook   ,&WR_CancelHook,winnum);
}

///
/// WR_New
//  Creates a write window
static struct WR_ClassData *WR_New(int winnum)
{
   struct WR_ClassData *data = calloc(1, sizeof(struct WR_ClassData));

   if(data)
   {
      enum {
        WMEN_NEW=501,WMEN_OPEN,WMEN_INSFILE,WMEN_SAVEAS,WMEN_INSQUOT,WMEN_INSALTQUOT,
        WMEN_INSROT13,WMEN_EDIT,WMEN_CUT,WMEN_COPY,WMEN_PASTE,WMEN_SELECTALL,
        WMEN_PASQUOT,WMEN_PASALTQUOT,WMEN_PASROT13,WMEN_SEARCH,WMEN_SEARCHAGAIN,WMEN_DICT,
        WMEN_STYLE_BOLD,WMEN_STYLE_ITALIC,WMEN_STYLE_UNDERLINE,
        WMEN_STYLE_COLORED,WMEN_EMOT0,WMEN_EMOT1,WMEN_EMOT2,WMEN_EMOT3,WMEN_UNDO,WMEN_REDO,
        WMEN_AUTOSP,WMEN_AUTOWRAP,WMEN_ADDFILE, WMEN_ADDCLIP, WMEN_ADDPGP,
        WMEN_DELSEND,WMEN_MDN,WMEN_ADDINFO,WMEN_IMPORT0,WMEN_IMPORT1,
        WMEN_IMPORT2,WMEN_SIGN0,WMEN_SIGN1,WMEN_SIGN2,WMEN_SIGN3,
        WMEN_SECUR0,WMEN_SECUR1,WMEN_SECUR2,WMEN_SECUR3,WMEN_SECUR4, WMEN_SECUR5, WMEN_INSUUCODE,
      };

      static const char *rtitles[4] = { NULL };
      static const char *encoding[3];
      static const char *security[SEC_MAXDUMMY+1];
      static const char *priority[4];
      static const char *signat[5];

      static const char *emoticons[4] =
      {
        ":-)", ":-|", ":-(", ";-)"
      };

      Object *sec_menus[SEC_MAXDUMMY];
      Object *mi_copy;
      Object *mi_cut;
      Object *mi_paste;
      Object *mi_redo;
      Object *mi_undo;
      Object *mi_selectall;
      Object *strip;
      Object *mi_autospell;
      Object *mi_autowrap;
      Object *mi_delsend;
      Object *mi_mdn;
      Object *mi_addinfo;
      Object *slider = ScrollbarObject, End;
      ULONG i;

      if(rtitles[0] == '\0')   // only initialize static data on first call
      {
         rtitles[0] = tr(MSG_Message);
         rtitles[1] = tr(MSG_Attachments);
         rtitles[2] = tr(MSG_Options);
         rtitles[3] = NULL;

         encoding[0] = "Base64/QP";
         encoding[1] = "UUencode";
         encoding[2] = NULL;

         security[SEC_NONE]    = tr(MSG_WR_SecNone);
         security[SEC_SIGN]    = tr(MSG_WR_SecSign);
         security[SEC_ENCRYPT] = tr(MSG_WR_SecEncrypt);
         security[SEC_BOTH]    = tr(MSG_WR_SecBoth);
         security[SEC_SENDANON]= tr(MSG_WR_SecAnon);
         security[SEC_DEFAULTS]= tr(MSG_WR_SecDefaults);
         security[SEC_MAXDUMMY]= NULL;

         priority[0] = tr(MSG_WR_ImpHigh);
         priority[1] = tr(MSG_WR_ImpNormal);
         priority[2] = tr(MSG_WR_ImpLow);
         priority[3] = NULL;

         signat[0] = tr(MSG_WR_NoSig);
         signat[1] = tr(MSG_WR_DefSig);
         signat[2] = tr(MSG_WR_AltSig1);
         signat[3] = tr(MSG_WR_AltSig2);
         signat[4] = NULL;
      }

      // set the winnum variable of the classdata
      data->winnum = winnum;

      // now go and create the window object
      data->GUI.WI = WindowObject,
         MUIA_Window_Title, tr(MSG_WR_WriteWT),
         MUIA_HelpNode, "WR_W",
         MUIA_Window_ID, MAKE_ID('W','R','I','T'),
         MUIA_Window_Menustrip, strip = MenustripObject,
            MUIA_Family_Child, MenuObject, MUIA_Menu_Title, tr(MSG_WR_Text),
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,tr(MSG_New), MUIA_Menuitem_Shortcut,"N", MUIA_UserData,WMEN_NEW, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,tr(MSG_Open), MUIA_Menuitem_Shortcut,"O", MUIA_UserData,WMEN_OPEN, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,tr(MSG_WR_InsertAs),
                  MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,tr(MSG_WR_Plain), MUIA_Menuitem_Shortcut,"P", MUIA_UserData,WMEN_INSFILE, End,
                  MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,tr(MSG_WR_Quoted), MUIA_UserData,WMEN_INSQUOT, End,
                  MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,tr(MSG_WR_AltQuoted), MUIA_UserData,WMEN_INSALTQUOT, End,
                  MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,tr(MSG_WR_ROT13), MUIA_UserData,WMEN_INSROT13, End,
                  MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,tr(MSG_WR_UUCODE), MUIA_UserData,WMEN_INSUUCODE, End,
               End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,NM_BARLABEL, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,tr(MSG_SaveAs), MUIA_UserData,WMEN_SAVEAS, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,NM_BARLABEL, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,tr(MSG_WR_LaunchEd), MUIA_Menuitem_Shortcut,"E", MUIA_UserData,WMEN_EDIT, End,
            End,
            MUIA_Family_Child, MenuObject, MUIA_Menu_Title, tr(MSG_WR_Edit),
               MUIA_Family_Child, mi_cut = MenuitemObject, MUIA_Menuitem_Title,tr(MSG_WR_MCut), MUIA_UserData, WMEN_CUT, End,
               MUIA_Family_Child, mi_copy = MenuitemObject, MUIA_Menuitem_Title,tr(MSG_WR_MCopy), MUIA_UserData, WMEN_COPY, End,
               MUIA_Family_Child, mi_paste = MenuitemObject, MUIA_Menuitem_Title,tr(MSG_WR_MPaste), MUIA_UserData, WMEN_PASTE, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,tr(MSG_WR_PasteAs),
                  MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,tr(MSG_WR_Quoted), MUIA_Menuitem_Shortcut,"Q", MUIA_UserData,WMEN_PASQUOT, End,
                  MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,tr(MSG_WR_AltQuoted), MUIA_UserData,WMEN_PASALTQUOT, End,
                  MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,tr(MSG_WR_ROT13), MUIA_UserData,WMEN_PASROT13, End,
               End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,NM_BARLABEL, End,
               MUIA_Family_Child, mi_undo = MenuitemObject, MUIA_Menuitem_Title,tr(MSG_WR_MUndo), MUIA_UserData, WMEN_UNDO, End,
               MUIA_Family_Child, mi_redo = MenuitemObject, MUIA_Menuitem_Title,tr(MSG_WR_Redo), MUIA_UserData, WMEN_REDO, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,NM_BARLABEL, End,
               MUIA_Family_Child, mi_selectall = MenuitemObject, MUIA_Menuitem_Title,tr(MSG_WR_SELECTALL), MUIA_UserData, WMEN_SELECTALL, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,NM_BARLABEL, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,tr(MSG_WR_SEARCH), MUIA_Menuitem_Shortcut,"F", MUIA_UserData,WMEN_SEARCH, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,tr(MSG_WR_SEARCH_AGAIN), MUIA_Menuitem_Shortcut,"G", MUIA_UserData,WMEN_SEARCHAGAIN, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,NM_BARLABEL, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,tr(MSG_WR_Dictionary), MUIA_Menuitem_Shortcut,"D", MUIA_UserData,WMEN_DICT, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,tr(MSG_WR_Textstyle),
                  MUIA_Family_Child, data->GUI.MI_BOLD = MenuitemObject, MUIA_Menuitem_Title,tr(MSG_WR_Bold), MUIA_Menuitem_Shortcut,"B", MUIA_Menuitem_Checkit,TRUE, MUIA_Menuitem_Toggle,TRUE, MUIA_UserData,WMEN_STYLE_BOLD, End,
                  MUIA_Family_Child, data->GUI.MI_ITALIC = MenuitemObject, MUIA_Menuitem_Title,tr(MSG_WR_Italic), MUIA_Menuitem_Shortcut,"I", MUIA_Menuitem_Checkit,TRUE, MUIA_Menuitem_Toggle,TRUE, MUIA_UserData,WMEN_STYLE_ITALIC, End,
                  MUIA_Family_Child, data->GUI.MI_UNDERLINE = MenuitemObject, MUIA_Menuitem_Title,tr(MSG_WR_Underlined), MUIA_Menuitem_Shortcut,"U", MUIA_Menuitem_Checkit,TRUE, MUIA_Menuitem_Toggle,TRUE, MUIA_UserData,WMEN_STYLE_UNDERLINE, End,
                  MUIA_Family_Child, data->GUI.MI_COLORED = MenuitemObject, MUIA_Menuitem_Title,tr(MSG_WR_Colored), MUIA_Menuitem_Shortcut,"H", MUIA_Menuitem_Checkit,TRUE, MUIA_Menuitem_Toggle,TRUE, MUIA_UserData,WMEN_STYLE_COLORED, End,
               End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,tr(MSG_WR_Emoticons),
                  MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,tr(MSG_WR_Happy), MUIA_UserData,WMEN_EMOT0, End,
                  MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,tr(MSG_WR_Indifferent), MUIA_UserData,WMEN_EMOT1, End,
                  MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,tr(MSG_WR_Sad), MUIA_UserData,WMEN_EMOT2, End,
                  MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,tr(MSG_WR_Ironic), MUIA_UserData,WMEN_EMOT3, End,
               End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,NM_BARLABEL, End,
               MUIA_Family_Child, mi_autospell = MenuitemObject, MUIA_Menuitem_Title, tr(MSG_WR_SpellCheck), MUIA_Menuitem_Checkit,TRUE, MUIA_Menuitem_Toggle,TRUE, MUIA_UserData,WMEN_AUTOSP, End,
               MUIA_Family_Child, mi_autowrap = MenuitemObject, MUIA_Menuitem_Title, tr(MSG_WR_AUTOWRAP), MUIA_Menuitem_Checkit,TRUE, MUIA_Menuitem_Toggle,TRUE, MUIA_UserData,WMEN_AUTOWRAP, End,
            End,
            MUIA_Family_Child, MenuObject, MUIA_Menu_Title, tr(MSG_Attachments),
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,tr(MSG_WR_MAddFile), MUIA_Menuitem_Shortcut,"L", MUIA_UserData,WMEN_ADDFILE, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,tr(MSG_WR_AddCB), MUIA_UserData,WMEN_ADDCLIP, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,tr(MSG_WR_AddKey), MUIA_UserData,WMEN_ADDPGP, End,
            End,
            MUIA_Family_Child, MenuObject, MUIA_Menu_Title, tr(MSG_Options),
               MUIA_Family_Child, mi_delsend = MenuitemObject, MUIA_Menuitem_Title, tr(MSG_WR_MDelSend), MUIA_Menuitem_Checkit, TRUE, MUIA_Menuitem_Toggle, TRUE, MUIA_UserData, WMEN_DELSEND, End,
               MUIA_Family_Child, mi_mdn =     MenuitemObject, MUIA_Menuitem_Title, tr(MSG_WR_MReceipt), MUIA_Menuitem_Checkit, TRUE, MUIA_Menuitem_Toggle, TRUE, MUIA_UserData, WMEN_MDN,     End,
               MUIA_Family_Child, mi_addinfo = MenuitemObject, MUIA_Menuitem_Title, tr(MSG_WR_MAddInfo), MUIA_Menuitem_Checkit, TRUE, MUIA_Menuitem_Toggle, TRUE, MUIA_UserData, WMEN_ADDINFO, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,tr(MSG_WR_MImportance),
                  MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,priority[0], MUIA_Menuitem_Checkit,TRUE, MUIA_Menuitem_Exclude,0x06, MUIA_UserData,WMEN_IMPORT0, End,
                  MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,priority[1], MUIA_Menuitem_Checkit,TRUE, MUIA_Menuitem_Exclude,0x05, MUIA_Menuitem_Checked,TRUE, MUIA_UserData,WMEN_IMPORT1, End,
                  MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,priority[2], MUIA_Menuitem_Checkit,TRUE, MUIA_Menuitem_Exclude,0x03, MUIA_UserData,WMEN_IMPORT2, End,
               End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,tr(MSG_CO_CrdSignature),
                  MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,signat[0], MUIA_Menuitem_Shortcut,"0", MUIA_Menuitem_Checkit,TRUE, MUIA_Menuitem_Exclude,0x0E, MUIA_Menuitem_Checked,!C->UseSignature, MUIA_UserData,WMEN_SIGN0, End,
                  MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,signat[1], MUIA_Menuitem_Shortcut,"7", MUIA_Menuitem_Checkit,TRUE, MUIA_Menuitem_Exclude,0x0D, MUIA_Menuitem_Checked,C->UseSignature, MUIA_UserData,WMEN_SIGN1, End,
                  MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,signat[2], MUIA_Menuitem_Shortcut,"8", MUIA_Menuitem_Checkit,TRUE, MUIA_Menuitem_Exclude,0x0B, MUIA_UserData,WMEN_SIGN2, End,
                  MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,signat[3], MUIA_Menuitem_Shortcut,"9", MUIA_Menuitem_Checkit,TRUE, MUIA_Menuitem_Exclude,0x07, MUIA_UserData,WMEN_SIGN3, End,
               End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,tr(MSG_CO_CrdSecurity),
                  MUIA_Family_Child, sec_menus[SEC_NONE]     = MenuitemObject, MUIA_Menuitem_Title,security[SEC_NONE]    , MUIA_Menuitem_Checkit,TRUE, MUIA_Menuitem_Exclude,0x3E, MUIA_UserData,WMEN_SECUR0, End,
                  MUIA_Family_Child, sec_menus[SEC_SIGN]     = MenuitemObject, MUIA_Menuitem_Title,security[SEC_SIGN]    , MUIA_Menuitem_Checkit,TRUE, MUIA_Menuitem_Exclude,0x3D, MUIA_UserData,WMEN_SECUR1, End,
                  MUIA_Family_Child, sec_menus[SEC_ENCRYPT]  = MenuitemObject, MUIA_Menuitem_Title,security[SEC_ENCRYPT] , MUIA_Menuitem_Checkit,TRUE, MUIA_Menuitem_Exclude,0x3B, MUIA_UserData,WMEN_SECUR2, End,
                  MUIA_Family_Child, sec_menus[SEC_BOTH]     = MenuitemObject, MUIA_Menuitem_Title,security[SEC_BOTH]    , MUIA_Menuitem_Checkit,TRUE, MUIA_Menuitem_Exclude,0x37, MUIA_UserData,WMEN_SECUR3, End,
                  MUIA_Family_Child, sec_menus[SEC_SENDANON] = MenuitemObject, MUIA_Menuitem_Title,security[SEC_SENDANON], MUIA_Menuitem_Checkit,TRUE, MUIA_Menuitem_Exclude,0x2F, MUIA_UserData,WMEN_SECUR4, End,
                  MUIA_Family_Child, sec_menus[SEC_DEFAULTS] = MenuitemObject, MUIA_Menuitem_Title,security[SEC_DEFAULTS], MUIA_Menuitem_Checkit,TRUE, MUIA_Menuitem_Exclude,0x1F, MUIA_UserData,WMEN_SECUR5, MUIA_Menuitem_Checked, TRUE, End,
               End,
            End,
         End,
         MUIA_Window_AppWindow, TRUE,
         WindowContents, VGroup,
            Child, data->GUI.RG_PAGE = RegisterGroup(rtitles),
               MUIA_CycleChain, 1,
               Child, VGroup, /* Message */
                  MUIA_HelpNode, "WR00",
                  Child, ColGroup(2),
                     Child, Label(tr(MSG_WR_To)),
                     Child, MakeAddressField(&data->GUI.ST_TO, tr(MSG_WR_To), MSG_HELP_WR_ST_TO, ABM_TO, winnum, TRUE),
                     Child, Label(tr(MSG_WR_Subject)),
                     Child, data->GUI.ST_SUBJECT = MakeString(SIZE_SUBJECT,tr(MSG_WR_Subject)),
                  End,
                  Child, hasHideToolBarFlag(C->HideGUIElements) ?
                     (RectangleObject, MUIA_ShowMe, FALSE, End) :
                     (HGroup, GroupSpacing(0),
                        Child, data->GUI.TO_TOOLBAR = WriteWindowToolbarObject,
                        End,
                        Child, hasHideXYFlag(C->HideGUIElements) ?
                           (HSpace(1)) :
                           (HGroup,
                             GroupSpacing(0),
                             Child, RectangleObject,
                               MUIA_Rectangle_VBar, TRUE,
                               MUIA_FixWidth,        3,
                             End,
                             Child, VGroup,
                               GroupSpacing(0),
                               Child, VSpace(0),
                               Child, data->GUI.TX_POSI = TextObject,
                                 MUIA_Weight,        0,
                                 MUIA_Text_Contents, "000 \n000 ",
                                 MUIA_Background,    MUII_RegisterBack,
                                 MUIA_Frame,         MUIV_Frame_None,
                                 MUIA_Font,          MUIV_Font_Tiny,
                               End,
                               Child, VSpace(0),
                             End,
                           End),
                     End),
                  Child, HGroup,
                     MUIA_HelpNode, "EDIT",
                     MUIA_Group_Spacing, 0,
                     Child, data->GUI.TE_EDIT = MailTextEditObject,
                        InputListFrame,
                        MUIA_CycleChain, TRUE,
                        MUIA_TextEditor_Slider, slider,
                        MUIA_TextEditor_FixedFont,  C->FixedFontEdit,
                        MUIA_TextEditor_WrapBorder, C->EdWrapMode == 1 ? C->EdWrapCol : 0,
                        MUIA_TextEditor_ExportWrap, C->EdWrapMode == 2 ? C->EdWrapCol : 0,
                        MUIA_TextEditor_ImportHook, MUIV_TextEditor_ImportHook_Plain,
                        MUIA_TextEditor_ExportHook, MUIV_TextEditor_ExportHook_NoStyle,
                     End,
                     Child, slider,
                  End,
               End,
               Child, VGroup, /* Attachments */
                  MUIA_HelpNode, "WR01",
                  Child, NListviewObject,
                     MUIA_CycleChain, 1,
                     MUIA_Listview_DragType,  MUIV_Listview_DragType_Immediate,
                     MUIA_NListview_NList,    data->GUI.LV_ATTACH = WriteAttachmentListObject,
                        InputListFrame,
                        MUIA_NList_ListBackground, MUII_ListBack,
                        MUIA_NList_TitleBackground, MUII_ListBack,
                        MUIA_NList_DragSortable ,TRUE,
                        MUIA_NList_Format       ,"D=8 BAR,P=\033r D=8 BAR,D=8 BAR,P=\033c D=8 BAR,",
                        MUIA_NList_Title        ,TRUE,
                        MUIA_NList_ConstructHook,&WR_LV_ConFuncHook,
                        MUIA_NList_DestructHook ,&GeneralDesHook,
                        MUIA_NList_DisplayHook  ,&WR_LV_DspFuncHook,
                     End,
                  End,
                  Child, ColGroup(4),
                     Child, data->GUI.BT_ADD     = MakeButton(tr(MSG_WR_Add)),
                     Child, data->GUI.BT_ADDPACK = MakeButton(tr(MSG_WR_AddPack)),
                     Child, data->GUI.BT_DEL     = MakeButton(tr(MSG_Del)),
                     Child, data->GUI.BT_DISPLAY = MakeButton(tr(MSG_WR_Display)),
                  End,
                  Child, HGroup,
                     Child, data->GUI.RA_ENCODING = RadioObject,
                        GroupFrameT(tr(MSG_WR_Encoding)),
                        MUIA_Radio_Entries, encoding,
                        MUIA_CycleChain, 1,
                     End,
                     Child, ColGroup(2),
                        Child, Label2(tr(MSG_WR_ContentType)),
                        Child, MakeMimeTypePop(&data->GUI.ST_CTYPE, tr(MSG_WR_ContentType)),
                        Child, Label2(tr(MSG_WR_Description)),
                        Child, data->GUI.ST_DESC = MakeString(SIZE_DEFAULT,tr(MSG_WR_Description)),
                        Child, HSpace(0),
                     End,
                  End,
               End,
               Child, VGroup, /* Options */
                  MUIA_HelpNode, "WR02",
                  Child, ColGroup(2),
                     Child, Label(tr(MSG_WR_CopyTo)),
                     Child, MakeAddressField(&data->GUI.ST_CC, tr(MSG_WR_CopyTo), MSG_HELP_WR_ST_CC, ABM_CC, winnum, TRUE),
                     Child, Label(tr(MSG_WR_BlindCopyTo)),
                     Child, MakeAddressField(&data->GUI.ST_BCC, tr(MSG_WR_BlindCopyTo), MSG_HELP_WR_ST_BCC, ABM_BCC, winnum, TRUE),
                     Child, Label(tr(MSG_WR_From)),
                     Child, MakeAddressField(&data->GUI.ST_FROM, tr(MSG_WR_From), MSG_HELP_WR_ST_FROM, ABM_FROM, winnum, TRUE),
                     Child, Label(tr(MSG_WR_ReplyTo)),
                     Child, MakeAddressField(&data->GUI.ST_REPLYTO, tr(MSG_WR_ReplyTo), MSG_HELP_WR_ST_REPLYTO, ABM_REPLYTO, winnum, TRUE),
                     Child, Label(tr(MSG_WR_ExtraHeaders)),
                     Child, data->GUI.ST_EXTHEADER = MakeString(SIZE_LARGE,tr(MSG_WR_ExtraHeaders)),
                  End,
                  Child, HGroup,
                     Child, VGroup, GroupFrameT(tr(MSG_WR_SendOpt)),
                        Child, MakeCheckGroup((Object **)&data->GUI.CH_DELSEND, tr(MSG_WR_DelSend)),
                        Child, MakeCheckGroup((Object **)&data->GUI.CH_MDN,     tr(MSG_WR_Receipt)),
                        Child, MakeCheckGroup((Object **)&data->GUI.CH_ADDINFO, tr(MSG_WR_AddInfo)),
                        Child, HGroup,
                           Child, Label(tr(MSG_WR_Importance)),
                           Child, data->GUI.CY_IMPORTANCE = MakeCycle(priority, tr(MSG_WR_Importance)),
                        End,
                     End,
                     Child, HSpace(0),
                     Child, data->GUI.RA_SIGNATURE = RadioObject, GroupFrameT(tr(MSG_WR_Signature)),
                        MUIA_Radio_Entries, signat,
                        MUIA_Radio_Active, C->UseSignature ? 1 : 0,
                        MUIA_CycleChain, 1,
                     End,
                     Child, HSpace(0),
                     Child, data->GUI.RA_SECURITY = RadioObject, GroupFrameT(tr(MSG_WR_Security)),
                        MUIA_Radio_Entries, security,
                        MUIA_Radio_Active, SEC_DEFAULTS,
                        MUIA_CycleChain, 1,
                     End,
                  End,
               End,
            End,
            Child, ColGroup(4),
               Child, data->GUI.BT_SEND   = MakeButton(tr(MSG_WR_Send)),
               Child, data->GUI.BT_QUEUE  = MakeButton(tr(MSG_WR_ToQueue)),
               Child, data->GUI.BT_HOLD   = MakeButton(tr(MSG_WR_Hold)),
               Child, data->GUI.BT_CANCEL = MakeButton(tr(MSG_Cancel)),
            End,
         End,
      End;

      if(data->GUI.WI && slider)
      {
         DoMethod(G->App, OM_ADDMEMBER, data->GUI.WI);

         SetAttrs(data->GUI.ST_TO,
                 MUIA_BetterString_KeyDownFocus, data->GUI.ST_SUBJECT,
                 MUIA_Recipientstring_FromString, data->GUI.ST_FROM,
                 MUIA_Recipientstring_ReplyToString, data->GUI.ST_REPLYTO,
                 TAG_DONE);
         SetAttrs(data->GUI.ST_SUBJECT,
                 MUIA_BetterString_KeyUpFocus, data->GUI.ST_TO,
                 MUIA_BetterString_KeyDownFocus, data->GUI.TE_EDIT,
                 TAG_DONE);

         set(mi_autospell, MUIA_Menuitem_Checked, xget(data->GUI.TE_EDIT, MUIA_TextEditor_TypeAndSpell));
         set(mi_autowrap,  MUIA_Menuitem_Checked, xget(data->GUI.TE_EDIT, MUIA_TextEditor_WrapBorder) > 0);

         set(data->GUI.CY_IMPORTANCE, MUIA_Cycle_Active, 1);
         DoMethod(G->App, MUIM_MultiSet,  MUIA_Disabled, TRUE, data->GUI.RA_ENCODING, data->GUI.ST_CTYPE, data->GUI.ST_DESC, data->GUI.BT_DEL, data->GUI.BT_DISPLAY, NULL);
         SetHelp(data->GUI.ST_SUBJECT    ,MSG_HELP_WR_ST_SUBJECT   );
         SetHelp(data->GUI.BT_ADD        ,MSG_HELP_WR_BT_ADD       );
         SetHelp(data->GUI.BT_ADDPACK    ,MSG_HELP_WR_BT_ADDPACK   );
         SetHelp(data->GUI.BT_DEL        ,MSG_HELP_WR_BT_DEL       );
         SetHelp(data->GUI.BT_DISPLAY    ,MSG_HELP_WR_BT_DISPLAY   );
         SetHelp(data->GUI.RA_ENCODING   ,MSG_HELP_WR_RA_ENCODING  );
         SetHelp(data->GUI.ST_CTYPE      ,MSG_HELP_WR_ST_CTYPE     );
         SetHelp(data->GUI.ST_DESC       ,MSG_HELP_WR_ST_DESC      );
         SetHelp(data->GUI.ST_EXTHEADER  ,MSG_HELP_WR_ST_EXTHEADER );
         SetHelp(data->GUI.CH_DELSEND    ,MSG_HELP_WR_CH_DELSEND   );
         SetHelp(data->GUI.CH_MDN,        MSG_HELP_WR_CH_RECEIPT   );
         SetHelp(data->GUI.CH_ADDINFO    ,MSG_HELP_WR_CH_ADDINFO   );
         SetHelp(data->GUI.CY_IMPORTANCE ,MSG_HELP_WR_CY_IMPORTANCE);
         SetHelp(data->GUI.RA_SIGNATURE  ,MSG_HELP_WR_RA_SIGNATURE );
         SetHelp(data->GUI.RA_SECURITY   ,MSG_HELP_WR_RA_SECURITY  );
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,WMEN_NEW       ,data->GUI.TE_EDIT      ,1,MUIM_TextEditor_ClearText);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,WMEN_OPEN      ,MUIV_Notify_Application,4,MUIM_CallHook   ,&WR_EditorCmdHook,ED_OPEN,winnum);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,WMEN_INSFILE   ,MUIV_Notify_Application,4,MUIM_CallHook   ,&WR_EditorCmdHook,ED_INSERT,winnum);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,WMEN_INSQUOT   ,MUIV_Notify_Application,4,MUIM_CallHook   ,&WR_EditorCmdHook,ED_INSQUOT,winnum);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,WMEN_INSALTQUOT,MUIV_Notify_Application,4,MUIM_CallHook   ,&WR_EditorCmdHook,ED_INSALTQUOT,winnum);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,WMEN_INSROT13  ,MUIV_Notify_Application,4,MUIM_CallHook   ,&WR_EditorCmdHook,ED_INSROT13,winnum);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,WMEN_INSUUCODE ,MUIV_Notify_Application,4,MUIM_CallHook   ,&WR_EditorCmdHook,ED_INSUUCODE,winnum);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,WMEN_SAVEAS    ,MUIV_Notify_Application,3,MUIM_CallHook   ,&WR_SaveAsHook,winnum);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,WMEN_EDIT      ,MUIV_Notify_Application,3,MUIM_CallHook   ,&WR_EditHook,winnum);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,WMEN_CUT       ,data->GUI.TE_EDIT      ,2,MUIM_TextEditor_ARexxCmd,"CUT");
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,WMEN_COPY      ,data->GUI.TE_EDIT      ,2,MUIM_TextEditor_ARexxCmd,"COPY");
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,WMEN_PASTE     ,data->GUI.TE_EDIT      ,2,MUIM_TextEditor_ARexxCmd,"PASTE");
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,WMEN_PASQUOT   ,MUIV_Notify_Application,4,MUIM_CallHook   ,&WR_EditorCmdHook,ED_PASQUOT,winnum);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,WMEN_PASALTQUOT,MUIV_Notify_Application,4,MUIM_CallHook   ,&WR_EditorCmdHook,ED_PASALTQUOT,winnum);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,WMEN_PASROT13  ,MUIV_Notify_Application,4,MUIM_CallHook   ,&WR_EditorCmdHook,ED_PASROT13,winnum);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,WMEN_SEARCH    ,MUIV_Notify_Application,4,MUIM_CallHook   ,&WR_SearchHook, data->GUI.TE_EDIT, MUIF_NONE);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,WMEN_SEARCHAGAIN,MUIV_Notify_Application,4,MUIM_CallHook  ,&WR_SearchHook, data->GUI.TE_EDIT, MUIF_ReadMailGroup_Search_Again);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,WMEN_DICT      ,MUIV_Notify_Application,3,MUIM_CallHook   ,&DI_OpenHook,winnum);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,WMEN_UNDO      ,data->GUI.TE_EDIT      ,2,MUIM_TextEditor_ARexxCmd,"UNDO");
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,WMEN_REDO      ,data->GUI.TE_EDIT      ,2,MUIM_TextEditor_ARexxCmd,"REDO");
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,WMEN_ADDFILE   ,MUIV_Notify_Application,3,MUIM_CallHook   ,&WR_AddFileHook,winnum);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,WMEN_ADDCLIP   ,MUIV_Notify_Application,3,MUIM_CallHook   ,&WR_AddClipboardHook,winnum);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,WMEN_ADDPGP    ,MUIV_Notify_Application,3,MUIM_CallHook   ,&WR_AddPGPKeyHook,winnum);
         DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,WMEN_SELECTALL, data->GUI.TE_EDIT,      2, MUIM_TextEditor_ARexxCmd, "SELECTALL");
         for (i = 0; i < 4; i++) DoMethod(data->GUI.WI,MUIM_Notify,MUIA_Window_MenuAction,WMEN_EMOT0+i,data->GUI.TE_EDIT,2,MUIM_TextEditor_InsertText,emoticons[i]);
         DoMethod(data->GUI.RG_PAGE    ,MUIM_Notify,MUIA_AppMessage          ,MUIV_EveryTime ,MUIV_Notify_Application,4,MUIM_CallHook   ,&WR_AppHook,MUIV_TriggerValue,winnum);
         DoMethod(data->GUI.TE_EDIT    ,MUIM_Notify,MUIA_TextEditor_AreaMarked,MUIV_EveryTime,MUIV_Notify_Application,6,MUIM_MultiSet   ,MUIA_Menuitem_Enabled,MUIV_TriggerValue,mi_copy,mi_cut,NULL);

         if(data->GUI.TE_EDIT)
         {
           struct MUIP_TextEditor_Keybinding *key;

           // now we find out the current key bindings of the TextEditor
           // object for the copy/paste/cut and so on actions

           // COPY action
           if((key = (struct MUIP_TextEditor_Keybinding *)DoMethod(data->GUI.TE_EDIT, MUIM_TextEditor_QueryKeyAction, MUIV_TextEditor_KeyAction_Copy)) &&
              key->code > 500 && hasFlag(key->qualifier, IEQUALIFIER_RCOMMAND))
           {
             static char shortcut[] = "ramiga X";

             if(hasFlag(key->qualifier, IEQUALIFIER_RSHIFT|IEQUALIFIER_LSHIFT))
               shortcut[7] = toupper(key->code-500);
             else
               shortcut[7] = key->code-500;

             SetAttrs(mi_copy, MUIA_Menuitem_CommandString, TRUE,
                               MUIA_Menuitem_Shortcut,      shortcut,
                               TAG_DONE);
           }

           // CUT action
           if((key = (struct MUIP_TextEditor_Keybinding *)DoMethod(data->GUI.TE_EDIT, MUIM_TextEditor_QueryKeyAction, MUIV_TextEditor_KeyAction_Cut)) &&
              key->code > 500 && hasFlag(key->qualifier, IEQUALIFIER_RCOMMAND))
           {
             static char shortcut[] = "ramiga X";

             if(hasFlag(key->qualifier, IEQUALIFIER_RSHIFT|IEQUALIFIER_LSHIFT))
               shortcut[7] = toupper(key->code-500);
             else
               shortcut[7] = key->code-500;

             SetAttrs(mi_cut, MUIA_Menuitem_CommandString, TRUE,
                              MUIA_Menuitem_Shortcut,      shortcut,
                              TAG_DONE);
           }

           // PASTE action
           if((key = (struct MUIP_TextEditor_Keybinding *)DoMethod(data->GUI.TE_EDIT, MUIM_TextEditor_QueryKeyAction, MUIV_TextEditor_KeyAction_Paste)) &&
              key->code > 500 && hasFlag(key->qualifier, IEQUALIFIER_RCOMMAND))
           {
             static char shortcut[] = "ramiga X";

             if(hasFlag(key->qualifier, IEQUALIFIER_RSHIFT|IEQUALIFIER_LSHIFT))
               shortcut[7] = toupper(key->code-500);
             else
               shortcut[7] = key->code-500;

             SetAttrs(mi_paste, MUIA_Menuitem_CommandString, TRUE,
                                MUIA_Menuitem_Shortcut,      shortcut,
                                TAG_DONE);
           }

           // UNDO action
           if((key = (struct MUIP_TextEditor_Keybinding *)DoMethod(data->GUI.TE_EDIT, MUIM_TextEditor_QueryKeyAction, MUIV_TextEditor_KeyAction_Undo)) &&
              key->code > 500 && hasFlag(key->qualifier, IEQUALIFIER_RCOMMAND))
           {
             static char shortcut[] = "ramiga X";

             if(hasFlag(key->qualifier, IEQUALIFIER_RSHIFT|IEQUALIFIER_LSHIFT))
               shortcut[7] = toupper(key->code-500);
             else
               shortcut[7] = key->code-500;

             SetAttrs(mi_undo, MUIA_Menuitem_CommandString, TRUE,
                               MUIA_Menuitem_Shortcut,      shortcut,
                               TAG_DONE);
           }

           // REDO action
           if((key = (struct MUIP_TextEditor_Keybinding *)DoMethod(data->GUI.TE_EDIT, MUIM_TextEditor_QueryKeyAction, MUIV_TextEditor_KeyAction_Redo)) &&
              key->code > 500 && hasFlag(key->qualifier, IEQUALIFIER_RCOMMAND))
           {
             static char shortcut[] = "ramiga X";

             if(hasFlag(key->qualifier, IEQUALIFIER_RSHIFT|IEQUALIFIER_LSHIFT))
               shortcut[7] = toupper(key->code-500);
             else
               shortcut[7] = key->code-500;

             SetAttrs(mi_redo, MUIA_Menuitem_CommandString, TRUE,
                               MUIA_Menuitem_Shortcut,      shortcut,
                               TAG_DONE);
           }

           // SELECTALL action
           if((key = (struct MUIP_TextEditor_Keybinding *)DoMethod(data->GUI.TE_EDIT, MUIM_TextEditor_QueryKeyAction, MUIV_TextEditor_KeyAction_SelectAll)) &&
              key->code > 500 && hasFlag(key->qualifier, IEQUALIFIER_RCOMMAND))
           {
             static char shortcut[] = "ramiga X";

             if(hasFlag(key->qualifier, IEQUALIFIER_RSHIFT|IEQUALIFIER_LSHIFT))
               shortcut[7] = toupper(key->code-500);
             else
               shortcut[7] = key->code-500;

             SetAttrs(mi_selectall, MUIA_Menuitem_CommandString, TRUE,
                                    MUIA_Menuitem_Shortcut,      shortcut,
                                    TAG_DONE);
           }
         }

         // set some notifications on the toolbar
         // in case it was generated
         if(data->GUI.TO_TOOLBAR)
           DoMethod(data->GUI.TO_TOOLBAR, MUIM_WriteWindowToolbar_InitNotify, data);

         DoMethod(data->GUI.TE_EDIT    ,MUIM_Notify,MUIA_TextEditor_UndoAvailable,MUIV_EveryTime,mi_undo            ,3,MUIM_Set,MUIA_Menuitem_Enabled,MUIV_TriggerValue);
         DoMethod(data->GUI.TE_EDIT    ,MUIM_Notify,MUIA_TextEditor_RedoAvailable,MUIV_EveryTime,mi_redo            ,3,MUIM_Set,MUIA_Menuitem_Enabled,MUIV_TriggerValue);
         if (data->GUI.TX_POSI)
         {
            DoMethod(data->GUI.TE_EDIT ,MUIM_Notify,MUIA_TextEditor_CursorX,MUIV_EveryTime,MUIV_Notify_Application,3,MUIM_CallHook,&WR_UpdateWTitleHook,winnum);
            DoMethod(data->GUI.TE_EDIT ,MUIM_Notify,MUIA_TextEditor_CursorY,MUIV_EveryTime,MUIV_Notify_Application,3,MUIM_CallHook,&WR_UpdateWTitleHook,winnum);
         }
         DoMethod(data->GUI.TE_EDIT    ,MUIM_Notify, MUIA_TextEditor_StyleBold,      MUIV_EveryTime, data->GUI.MI_BOLD, 3,MUIM_NoNotifySet, MUIA_Menuitem_Checked, MUIV_TriggerValue);
         DoMethod(data->GUI.TE_EDIT    ,MUIM_Notify, MUIA_TextEditor_StyleItalic,    MUIV_EveryTime, data->GUI.MI_ITALIC, 3,MUIM_NoNotifySet, MUIA_Menuitem_Checked, MUIV_TriggerValue);
         DoMethod(data->GUI.TE_EDIT    ,MUIM_Notify, MUIA_TextEditor_StyleUnderline, MUIV_EveryTime, data->GUI.MI_UNDERLINE, 3,MUIM_NoNotifySet, MUIA_Menuitem_Checked, MUIV_TriggerValue);
         DoMethod(data->GUI.TE_EDIT    ,MUIM_Notify, MUIA_TextEditor_Pen,            7,              data->GUI.MI_COLORED, 3,MUIM_NoNotifySet, MUIA_Menuitem_Checked, TRUE);
         DoMethod(data->GUI.TE_EDIT    ,MUIM_Notify, MUIA_TextEditor_Pen,            0,              data->GUI.MI_COLORED, 3,MUIM_NoNotifySet, MUIA_Menuitem_Checked, FALSE);
         DoMethod(data->GUI.MI_BOLD,    MUIM_Notify, MUIA_Menuitem_Checked,          MUIV_EveryTime, data->GUI.TE_EDIT, 4, MUIM_CallHook, &WR_SetSoftStyleHook, SSM_BOLD, winnum);
         DoMethod(data->GUI.MI_ITALIC,  MUIM_Notify, MUIA_Menuitem_Checked,          MUIV_EveryTime, data->GUI.TE_EDIT, 4, MUIM_CallHook, &WR_SetSoftStyleHook, SSM_ITALIC, winnum);
         DoMethod(data->GUI.MI_UNDERLINE, MUIM_Notify, MUIA_Menuitem_Checked,          MUIV_EveryTime, data->GUI.TE_EDIT, 4, MUIM_CallHook, &WR_SetSoftStyleHook, SSM_UNDERLINE, winnum);
         DoMethod(data->GUI.MI_COLORED, MUIM_Notify, MUIA_Menuitem_Checked,          MUIV_EveryTime, data->GUI.TE_EDIT, 4, MUIM_CallHook, &WR_SetSoftStyleHook, SSM_COLOR, winnum);
         DoMethod(data->GUI.RG_PAGE    ,MUIM_Notify,MUIA_Group_ActivePage    ,0             ,MUIV_Notify_Window     ,3,MUIM_Set        ,MUIA_Window_NoMenus,FALSE);
         DoMethod(data->GUI.RG_PAGE    ,MUIM_Notify,MUIA_Group_ActivePage    ,1             ,MUIV_Notify_Window     ,3,MUIM_Set        ,MUIA_Window_NoMenus,TRUE);
         DoMethod(data->GUI.RG_PAGE    ,MUIM_Notify,MUIA_Group_ActivePage    ,2             ,MUIV_Notify_Window     ,3,MUIM_Set        ,MUIA_Window_NoMenus,TRUE);
         DoMethod(data->GUI.ST_SUBJECT ,MUIM_Notify,MUIA_String_Acknowledge  ,MUIV_EveryTime,MUIV_Notify_Window     ,3,MUIM_Set        ,MUIA_Window_ActiveObject,data->GUI.TE_EDIT);
         DoMethod(data->GUI.BT_ADD     ,MUIM_Notify,MUIA_Pressed             ,FALSE         ,MUIV_Notify_Application,3,MUIM_CallHook   ,&WR_AddFileHook,winnum);
         DoMethod(data->GUI.BT_ADDPACK ,MUIM_Notify,MUIA_Pressed             ,FALSE         ,MUIV_Notify_Application,3,MUIM_CallHook   ,&WR_AddArchiveHook,winnum);
         DoMethod(data->GUI.BT_DEL     ,MUIM_Notify,MUIA_Pressed             ,FALSE         ,MUIV_Notify_Application,3,MUIM_CallHook   ,&WR_DeleteFileHook,winnum);
         DoMethod(data->GUI.BT_DISPLAY ,MUIM_Notify,MUIA_Pressed             ,FALSE         ,MUIV_Notify_Application,3,MUIM_CallHook   ,&WR_DisplayFileHook,winnum);
         DoMethod(data->GUI.LV_ATTACH  ,MUIM_Notify,MUIA_NList_Active        ,MUIV_EveryTime,MUIV_Notify_Application,3,MUIM_CallHook   ,&WR_GetFileEntryHook,winnum);
         DoMethod(data->GUI.RA_ENCODING,MUIM_Notify,MUIA_Radio_Active        ,MUIV_EveryTime,MUIV_Notify_Application,3,MUIM_CallHook   ,&WR_PutFileEntryHook,winnum);
         DoMethod(data->GUI.ST_CTYPE   ,MUIM_Notify,MUIA_String_Contents     ,MUIV_EveryTime,MUIV_Notify_Application,3,MUIM_CallHook   ,&WR_PutFileEntryHook,winnum);
         DoMethod(data->GUI.ST_DESC    ,MUIM_Notify,MUIA_String_Contents     ,MUIV_EveryTime,MUIV_Notify_Application,3,MUIM_CallHook   ,&WR_PutFileEntryHook,winnum);
         DoMethod(data->GUI.RA_SIGNATURE,MUIM_Notify,MUIA_Radio_Active        ,MUIV_EveryTime,MUIV_Notify_Application,4,MUIM_CallHook   ,&WR_ChangeSignatureHook,MUIV_TriggerValue,winnum);
         DoMethod(data->GUI.CH_DELSEND ,MUIM_Notify,MUIA_Selected            ,MUIV_EveryTime,mi_delsend              ,3,MUIM_Set        ,MUIA_Menuitem_Checked,MUIV_TriggerValue);
         DoMethod(data->GUI.CH_MDN,     MUIM_Notify,MUIA_Selected            ,MUIV_EveryTime,mi_mdn                  ,3,MUIM_Set        ,MUIA_Menuitem_Checked,MUIV_TriggerValue);
         DoMethod(data->GUI.CH_ADDINFO ,MUIM_Notify,MUIA_Selected            ,MUIV_EveryTime,mi_addinfo              ,3,MUIM_Set        ,MUIA_Menuitem_Checked,MUIV_TriggerValue);
         DoMethod(mi_autospell         ,MUIM_Notify,MUIA_Menuitem_Checked    ,MUIV_EveryTime,data->GUI.TE_EDIT       ,3,MUIM_Set        ,MUIA_TextEditor_TypeAndSpell,MUIV_TriggerValue);
         DoMethod(mi_autowrap          ,MUIM_Notify,MUIA_Menuitem_Checked    ,TRUE,          data->GUI.TE_EDIT       ,3,MUIM_Set        ,MUIA_TextEditor_WrapBorder, C->EdWrapCol);
         DoMethod(mi_autowrap          ,MUIM_Notify,MUIA_Menuitem_Checked    ,FALSE,         data->GUI.TE_EDIT       ,3,MUIM_Set        ,MUIA_TextEditor_WrapBorder, 0);
         DoMethod(mi_delsend           ,MUIM_Notify,MUIA_Menuitem_Checked    ,MUIV_EveryTime,data->GUI.CH_DELSEND    ,3,MUIM_Set        ,MUIA_Selected,MUIV_TriggerValue);
         DoMethod(mi_mdn,               MUIM_Notify, MUIA_Menuitem_Checked, MUIV_EveryTime, data->GUI.CH_MDN, 3, MUIM_Set, MUIA_Selected,MUIV_TriggerValue);
         DoMethod(mi_addinfo           ,MUIM_Notify,MUIA_Menuitem_Checked    ,MUIV_EveryTime,data->GUI.CH_ADDINFO    ,3,MUIM_Set        ,MUIA_Selected,MUIV_TriggerValue);
         DoMethod(data->GUI.RA_SECURITY,MUIM_Notify,MUIA_Radio_Active        ,4             ,data->GUI.RA_SIGNATURE  ,3,MUIM_Set        ,MUIA_Radio_Active,0);
         DoMethod(data->GUI.RA_SECURITY,MUIM_Notify,MUIA_Radio_Active        ,4             ,data->GUI.CH_ADDINFO    ,3,MUIM_Set        ,MUIA_Selected,FALSE);
         for (i = 0; i < 3; i++)
         {
            DoMethod(data->GUI.CY_IMPORTANCE,MUIM_Notify,MUIA_Cycle_Active     ,i              ,strip                  ,4,MUIM_SetUData,WMEN_IMPORT0+i,MUIA_Menuitem_Checked,TRUE);
            DoMethod(data->GUI.WI           ,MUIM_Notify,MUIA_Window_MenuAction,WMEN_IMPORT0+i ,data->GUI.CY_IMPORTANCE,3,MUIM_Set     ,MUIA_Cycle_Active,i);
         }
         for (i = 0; i < 4; i++)
         {
            DoMethod(data->GUI.RA_SIGNATURE ,MUIM_Notify,MUIA_Radio_Active     ,i              ,strip                  ,4,MUIM_SetUData,WMEN_SIGN0+i,MUIA_Menuitem_Checked,TRUE);
            DoMethod(data->GUI.WI           ,MUIM_Notify,MUIA_Window_MenuAction,WMEN_SIGN0+i   ,data->GUI.RA_SIGNATURE ,3,MUIM_Set     ,MUIA_Radio_Active,i);
         }
         for (i = SEC_NONE; i < SEC_MAXDUMMY; i++)
         {
            // connect menuitems -> radiobuttons
            DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction,WMEN_SECUR0+i,data->GUI.RA_SECURITY,3,MUIM_Set        ,MUIA_Radio_Active    ,i);
            // ...and the other way round
            DoMethod(data->GUI.RA_SECURITY,MUIM_Notify,MUIA_Radio_Active     ,i            ,sec_menus[i]         ,3,MUIM_NoNotifySet,MUIA_Menuitem_Checked,TRUE);
         }
         WR_SharedSetup(data, winnum);
         return data;
      }
      free(data);
   }
   return NULL;
}

///
/// WR_NewBounce
//  Creates a bounce window
static struct WR_ClassData *WR_NewBounce(int winnum)
{
   struct WR_ClassData *data = calloc(1, sizeof(struct WR_ClassData));
   if (data)
   {
      data->GUI.WI = WindowObject,
         MUIA_Window_Title, tr(MSG_WR_BounceWT),
         MUIA_HelpNode, "WR_W",
         MUIA_Window_ID, MAKE_ID('W','R','I','B'),
         WindowContents, VGroup,
            Child, ColGroup(2),
               Child, Label2(tr(MSG_WR_BounceTo)),
               Child, MakeAddressField(&data->GUI.ST_TO, tr(MSG_WR_BounceTo), MSG_HELP_WR_ST_TO, ABM_TO, winnum, TRUE),
            End,
            Child, ColGroup(4),
               Child, data->GUI.BT_SEND   = MakeButton(tr(MSG_WR_Send)),
               Child, data->GUI.BT_QUEUE  = MakeButton(tr(MSG_WR_ToQueue)),
               Child, data->GUI.BT_HOLD   = MakeButton(tr(MSG_WR_Hold)),
               Child, data->GUI.BT_CANCEL = MakeButton(tr(MSG_Cancel)),
            End,
         End,
      End;
      if (data->GUI.WI)
      {
         DoMethod(G->App, OM_ADDMEMBER, data->GUI.WI);
         WR_SharedSetup(data, winnum);
         return data;
      }
      free(data);
   }
   return NULL;
}
///
