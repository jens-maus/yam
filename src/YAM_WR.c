/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2008 by YAM Open Source Team

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
#include <stdlib.h>

#include <clib/alib_protos.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/muimaster.h>

#include "extrasrc.h"

#include "YAM.h"
#include "YAM_addressbook.h"
#include "YAM_addressbookEntry.h"
#include "YAM_utilities.h"
#include "YAM_config.h"
#include "YAM_error.h"
#include "YAM_stringsizes.h"
#include "YAM_mainFolder.h"
#include "YAM_global.h"

#include "classes/Classes.h"

#include "FileInfo.h"
#include "Locale.h"
#include "MailList.h"
#include "Mime.h"
#include "MUIObjects.h"

#include "Debug.h"

/**************************************************************************/

/*** Compose Message ***/
/// GetDateTime
//  Formats current date and time for Date header field
static char *GetDateTime(void)
{
  static char dt[SIZE_DEFAULT];

  ENTER();

  DateStamp2RFCString(dt, sizeof(dt), NULL, C->TimeZone + (C->DaylightSaving ? 60 : 0), FALSE);

  RETURN(dt);
  return dt;
}

///
/// NewMessageID
//  Creates a unique id, used for Message-ID header field
static char *NewMessageID(void)
{
  static char idbuf[SIZE_MSGID];
  ULONG seconds;
  struct DateStamp ds;

  ENTER();

  // lets calculate the seconds
  DateStamp(&ds);
  seconds = ds.ds_Days * 24 * 60 * 60 + ds.ds_Minute * 60; // seconds since 1-Jan-78

  // Here we try to generate a unique MessageID.
  // We try to be as much conform to the Recommandations for generating
  // unique Message IDs as we can: http://www.jwz.org/doc/mid.html
  snprintf(idbuf, sizeof(idbuf), "<%lx%lx.%lx@%s>", seconds, ds.ds_Tick, (ULONG)rand(), C->SMTP_Server);

  RETURN(idbuf);
  return idbuf;
}

///
/// NewBoundaryID
//  Creates a unique id, used for the MIME boundaries
static char *NewBoundaryID(void)
{
  static char idbuf[SIZE_MSGID];
  static int ctr = 0;

  ENTER();

  // Generate a unique Boundary ID which conforms to RFC 2045 and includes
  // a "=_" sequence to make it safe for quoted printable encoded parts
  snprintf(idbuf, sizeof(idbuf), "--=_BOUNDARY.%lx%lx.%02x", (ULONG)FindTask(NULL), (ULONG)rand(), ++ctr);

  RETURN(idbuf);
  return idbuf;
}

///
/// NewMIMEpart
//  Initializes a new message part
struct WritePart *NewMIMEpart(struct WriteMailData *wmData)
{
  struct WritePart *p;

  ENTER();

  if((p = calloc(1, sizeof(struct WritePart))) != NULL)
  {
    p->ContentType = "text/plain";
    p->EncType = ENC_7BIT;

    if(wmData != NULL)
    {
      p->Filename = wmData->filename;
      p->charset = wmData->charset;
    }
    else
      p->charset = G->localCharset;
  }
  else
    E(DBF_MAIL, "couldn't create new MIME part");

  RETURN(p);
  return p;
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
static void HeaderFputs(FILE *fh, const char *s, const char *param, const int offset)
{
  BOOL doEncoding = FALSE;
  char *c = (char *)s;
  int paramLen = 0;

  ENTER();

  if(param != NULL)
    paramLen = strlen(param);

  // let us now search for any non-ascii compliant character aswell
  // as converting each character with the translation table
  while(*c != '\0')
  {
    // check for any non-ascii character or
    // if the string would be
    if(!isascii(*c) || iscntrl(*c))
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
    {
      if((c-s+1+paramLen+6) > 78)
        doEncoding = TRUE;
    }
    else
    {
      // we have to check for stray =? strings as we are
      // going to consider a rfc2047 encoding
      if((c = strstr(s, "=?")) != NULL && isascii(*(c+1)) &&
         (c == s || isspace(*(c-1))))
      {
        doEncoding = TRUE;
      }
    }

  }

  // if an encoding is required, we go and process it accordingly but
  // have to check whether we do rfc2047 or rfc2231 based encoding
  if(doEncoding == TRUE)
  {
    if(param != NULL)
    {
      // do the actual rfc2231 based MIME paramater encoding
      D(DBF_MAIL, "writing RFC2231 content '%s'='%s'", param, s);
      rfc2231_encode_file(fh, param, s);
    }
    else
    {
      // do the actual rfc2047 based encoding
      D(DBF_MAIL, "writing RFC2047 content '%s' with offset %ld", s, offset);
      rfc2047_encode_file(fh, s, offset);
    }
  }
  else if(param != NULL)
  {
    D(DBF_MAIL, "writing quoted content '%s'='%s'", param, s);
    // output the parameter name right before
    // the resulting parameter value
    fprintf(fh, "\n\t%s=\"%s\"", param, s);
  }
  else
  {
    size_t len = strlen(s);

    // there seems to be non "violating" characters in the string and
    // the resulting string will also be not > 78 chars in case we
    // have to encode a MIME parameter, so we go and output the source
    // string immediately
    D(DBF_MAIL, "writing plain content '%s'", s);

    // all we have to make sure now is that we don't write longer lines
    // than 78 chars or we fold them
    if(len >= (size_t)75-offset)
    {
      char *p = (char *)s;
      char *e = (char *)s;
      char *last_space = NULL;
      size_t c = offset;

      // start our search
      while(len > 0)
      {
        if(*e == ' ')
          last_space = e;

        // check if we need a newline and
        // if so we go and write out the last
        // stuff including a newline.
        if(c >= 75 && last_space != NULL)
        {
          fwrite(p, last_space-p, 1, fh);

          if(len > 1)
            fwrite("\n ", 2, 1, fh);

          p = last_space+1;
          c = e-p;
          last_space = NULL;
        }

        c++;
        e++;
        len--;
      }

      if(c > 0)
        fwrite(p, e-p, 1, fh);
    }
    else
      fwrite(s, len, 1, fh);
  }

  LEAVE();
}

///
/// EmitHeader
//  Outputs a complete header line
void EmitHeader(FILE *fh, const char *hdr, const char *body)
{
  int offset;

  ENTER();

  D(DBF_MAIL, "writing header '%s' with content '%s'", hdr, body);

  offset = fprintf(fh, "%s: ", hdr);
  HeaderFputs(fh, body, NULL, offset);
  fputc('\n', fh);

  LEAVE();
}

///
/// EmitRcptField
//  Outputs the value of a recipient header line, one entry per line
static void EmitRcptField(FILE *fh, const char *body)
{
  char *bodycpy;

  ENTER();

  if((bodycpy = strdup(body)) != NULL)
  {
    char *part = bodycpy;

    while(part != NULL)
    {
      char *next;

      if(*part == '\0')
        break;

      if((next = MyStrChr(part, ',')))
        *next++ = '\0';

      HeaderFputs(fh, Trim(part), NULL, 0);

      if((part = next))
        fputs(",\n\t", fh);
    }

    free(bodycpy);
  }

  LEAVE();
}

///
/// EmitRcptHeader
//  Outputs a complete recipient header line
static void EmitRcptHeader(FILE *fh, const char *hdr, const char *body)
{
  ENTER();

  fprintf(fh, "%s: ", hdr);
  EmitRcptField(fh, body ? body : "");
  fputc('\n', fh);

  LEAVE();
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
  if(part->EncType != ENC_7BIT && strncmp(part->ContentType, "text/", 5) == 0)
    fprintf(fh, "; charset=%s", strippedCharsetName(part->charset));

  // output the "name" and Content-Disposition as well
  // as the "filename" parameter to the mail
  if((p = part->Name) && *p)
  {
    fputc(';', fh);
    HeaderFputs(fh, p, "name", 0); // output and do rfc2231 encoding
    fputs("\nContent-Disposition: attachment;", fh);
    HeaderFputs(fh, p, "filename", 0); // output and do rfc2231 encoding
  }
  fputc('\n', fh);

  // output the Content-Transfer-Encoding:
  if(part->EncType != ENC_7BIT)
  {
    const char *enc = "7bit";

    switch(part->EncType)
    {
      case ENC_B64:  enc = "base64"; break;
      case ENC_QP:   enc = "quoted-printable"; break;
      case ENC_UUE:  enc = "x-uue"; break;
      case ENC_8BIT: enc = "8bit"; break;
      case ENC_BIN:  enc = "binary"; break;
      case ENC_7BIT:
        // nothing
      break;
    }

    fprintf(fh, "Content-Transfer-Encoding: %s\n", enc);
  }

  // output the Content-Description if appropriate
  if((p = part->Description) != NULL && p[0] != '\0')
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

  ENTER();

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

  if(ab != NULL || C->MyPictureURL[0] != '\0')
  {
    fputs("X-SenderInfo: 1", fh);
    if(C->MyPictureURL[0] != '\0')
    {
      fputc(';', fh);
      HeaderFputs(fh, C->MyPictureURL, "picture", 0);
    }

    if(ab != NULL)
    {
      if(*ab->Homepage)
      {
        fputc(';', fh);
        HeaderFputs(fh, ab->Homepage, "homepage", 0);
      }
      if(*ab->Street)
      {
        fputc(';', fh);
        HeaderFputs(fh, ab->Street, "street", 0);
      }
      if(*ab->City)
      {
        fputc(';', fh);
        HeaderFputs(fh, ab->City, "city", 0);
      }
      if(*ab->Country)
      {
        fputc(';', fh);
        HeaderFputs(fh, ab->Country, "country", 0);
      }
      if(*ab->Phone)
      {
        fputc(';', fh);
        HeaderFputs(fh, ab->Phone, "phone", 0);
      }
      if(ab->BirthDay)
        fprintf(fh, ";\n\tdob=%ld", ab->BirthDay);
    }
    fputc('\n', fh);
  }

  LEAVE();
}
///
/// EncodePart
//  Encodes a message part
static BOOL EncodePart(FILE *ofh, const struct WritePart *part)
{
  BOOL result = FALSE;
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
        if(base64encode_file(ifh, ofh, convLF) > 0)
          result = TRUE;
        else
          ER_NewError(tr(MSG_ER_B64FILEENCODE), part->Filename);
      }
      break;

      case ENC_QP:
      {
        if(qpencode_file(ifh, ofh) >= 0)
          result = TRUE;
        else
          ER_NewError(tr(MSG_ER_QPFILEENCODE), part->Filename);
      }
      break;

      case ENC_UUE:
      {
        LONG size;

        ObtainFileInfo(part->Filename, FI_SIZE, &size);

        fprintf(ofh, "begin 644 %s\n", *part->Name ? part->Name : (char *)FilePart(part->Filename));

        if(uuencode_file(ifh, ofh) >= 0)
          result = TRUE;
        else
          ER_NewError(tr(MSG_ER_UUFILEENCODE), part->Filename);

        fprintf(ofh, "``\nend\nsize %ld\n", size);
      }
      break;

      default:
      {
        if(CopyFile(NULL, ofh, NULL, ifh) == TRUE)
          result = TRUE;
        else
          ER_NewError(tr(MSG_ER_FILEENCODE), part->Filename);
      }
      break;
    }

    fclose(ifh);
  }
  else
    ER_NewError(tr(MSG_ER_FILEENCODE), part->Filename);

  RETURN(result);
  return result;
}

///
/// WR_Anonymize
//  Inserts recipient header field for remailer service
static void WR_Anonymize(FILE *fh, char *body)
{
  char *ptr;

  ENTER();

  for(ptr = C->RMCommands; *ptr; ptr++)
  {
    if(ptr[0] == '\\' && ptr[1] == 'n')
    {
      ptr++;
      fputs("\n", fh);
    }
    else if(ptr[0] == '%' && ptr[1] == 's')
    {
      ptr++;
      EmitRcptField(fh, body);
    }
    else
      fputc(*ptr, fh);
  }
  fputs("\n", fh);

  LEAVE();
}

///
/// WR_GetPGPId
//  Gets PGP key id for a person
static char *WR_GetPGPId(struct Person *pe)
{
  char *pgpid = NULL;
  struct ABEntry *ab = NULL;

  ENTER();

  if(AB_SearchEntry(pe->RealName, ASM_REALNAME|ASM_USER, &ab) == 0)
  {
    AB_SearchEntry(pe->Address, ASM_ADDRESS|ASM_USER, &ab);
  }

  if(ab != NULL)
  {
    if(ab->PGPId[0] != '\0')
      pgpid = ab->PGPId;
  }

  RETURN(pgpid);
  return pgpid;
}
///
/// WR_GetPGPIds
//  Collects PGP key ids for all persons in a recipient field
static char *WR_GetPGPIds(char *source, char *ids)
{
  char *next;

  ENTER();

  for(; source; source = next)
  {
    char *pid;
    struct Person pe;

    if(source[0] == '\0')
      break;

    if((next = MyStrChr(source, ',')) != NULL)
      *next++ = 0;

    ExtractAddress(source, &pe);
    if((pid = WR_GetPGPId(&pe)) == NULL)
    {
      pid = pe.RealName[0] ? pe.RealName : pe.Address;
      ER_NewError(tr(MSG_ER_ErrorNoPGPId), source, pid);
    }
    ids = StrBufCat(ids, (G->PGPVersion == 5) ? "-r \"" : "\"");
    ids = StrBufCat(ids, pid);
    ids = StrBufCat(ids, "\" ");
  }

  RETURN(ids);
  return ids;
}
///
/// WR_Bounce
//  Bounce message: inserts resent-headers while copying the message
static BOOL WR_Bounce(FILE *fh, struct Compose *comp)
{
  BOOL result = FALSE;
  FILE *oldfh;

  ENTER();

  if(comp->refMail != NULL &&
     (oldfh = fopen(GetMailFile(NULL, NULL, comp->refMail), "r")))
  {
    char address[SIZE_LARGE];

    setvbuf(oldfh, NULL, _IOFBF, SIZE_FILEBUF);

    // now we add the "Resent-#?" type headers which are defined
    // by RFC2822 section 3.6.6. The RFC defined that these headers
    // should be added to the top of a message
    EmitHeader(fh, "Resent-From", BuildAddress(address, sizeof(address), C->EmailAddress, C->RealName));
    EmitHeader(fh, "Resent-Date", GetDateTime());
    EmitRcptHeader(fh, "Resent-To", comp->MailTo);
    EmitHeader(fh, "Resent-Message-ID", NewMessageID());

    // now we copy the rest of the message
    // directly from the file handlers
    result = CopyFile(NULL, fh, NULL, oldfh);

    fclose(oldfh);
  }

  RETURN(result);
  return result;
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
  ENTER();

  if(comp->ExtHeader[0] != '\0')
  {
    char *p;
    char ch = '\n';

    for(p = comp->ExtHeader; *p; ++p)
    {
      if(ch == '\n')
        fputs("X-YAM-Header-", fh);
      if(*p != '\\')
        ch = *p;
      else if(*++p == '\\')
        ch = '\\';
      else if(*p == 'n')
        ch = '\n';
      fputc(ch, fh);
    }
    if(ch != '\n')
      fputc('\n', fh);
  }

  LEAVE();
}

///
/// WR_ComposeReport
//  Assembles the parts of a message disposition notification
static const char *const MIMEwarn =
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
static const char *const PGPwarn  =
  "The following body part contains a PGP encrypted message. Either\n"
  "your mail reader doesn't support MIME/PGP as specified in RFC 2015,\n"
  "or the message was encrypted for someone else. To read the encrypted\n"
  "message, run the next body part through Pretty Good Privacy (PGP).\n\n";

static BOOL WR_ComposeReport(FILE *fh, const struct Compose *comp, const char *boundary)
{
  struct WritePart *p;

  ENTER();

  fprintf(fh, "Content-type: multipart/report; report-type=disposition-notification; boundary=\"%s\"\n\n", boundary);

  for(p = comp->FirstPart; p; p = p->Next)
  {
    fprintf(fh, "\n--%s\n", boundary);
    WriteContentTypeAndEncoding(fh, p);
    fputs("\n", fh);

    if(EncodePart(fh, p) == FALSE)
    {
      RETURN(FALSE);
      return FALSE;
    }
  }
  fprintf(fh, "\n--%s--\n\n", boundary);

  RETURN(TRUE);
  return TRUE;
}

///
/// WR_ComposePGP
//  Creates a signed and/or encrypted PGP/MIME message
static BOOL WR_ComposePGP(FILE *fh, struct Compose *comp, char *boundary)
{
  enum Security sec = comp->Security;
  BOOL success = FALSE;
  struct WritePart pgppart;
  char *ids = AllocStrBuf(SIZE_DEFAULT);
  char pgpfile[SIZE_PATHFILE];
  struct TempFile *tf2;

  ENTER();

  pgpfile[0] = '\0';

  pgppart.Filename = pgpfile;
  pgppart.EncType = ENC_7BIT;
  if(sec == SEC_ENCRYPT || sec == SEC_BOTH)
  {
    if(comp->MailTo)
      ids = WR_GetPGPIds(comp->MailTo, ids);
    if(comp->MailCC)
      ids = WR_GetPGPIds(comp->MailCC, ids);
    if(comp->MailBCC)
      ids = WR_GetPGPIds(comp->MailBCC, ids);

    if(C->EncryptToSelf == TRUE && C->MyPGPID[0] != '\0')
    {
      if(G->PGPVersion == 5)
        ids = StrBufCat(ids, "-r ");
      ids = StrBufCat(ids, C->MyPGPID);
    }
  }

  if((tf2 = OpenTempFile(NULL)) != NULL)
  {
    struct TempFile *tf;

    if((tf = OpenTempFile("w")) != NULL)
    {
       struct WritePart *firstpart = comp->FirstPart;

       WriteContentTypeAndEncoding(tf->FP, firstpart);
       fputc('\n', tf->FP);

       if(EncodePart(tf->FP, firstpart) == FALSE)
       {
         CloseTempFile(tf);
         CloseTempFile(tf2);

         RETURN(FALSE);
         return FALSE;
       }

       fclose(tf->FP);
       tf->FP = NULL;
       ConvertCRLF(tf->Filename, tf2->Filename, TRUE);
       CloseTempFile(tf);

       snprintf(pgpfile, sizeof(pgpfile), "%s.asc", tf2->Filename);

       if(sec == SEC_SIGN || sec == SEC_BOTH)
         PGPGetPassPhrase();

       switch(sec)
       {
         case SEC_SIGN:
         {
           char options[SIZE_LARGE];

           fprintf(fh, "Content-type: multipart/signed; boundary=\"%s\"; micalg=pgp-md5; protocol=\"application/pgp-signature\"\n\n%s\n--%s\n", boundary, MIMEwarn, boundary);
           WriteContentTypeAndEncoding(fh, firstpart);
           fputc('\n', fh);

           if(EncodePart(fh, firstpart) == FALSE)
           {
             CloseTempFile(tf2);

             RETURN(FALSE);
             return FALSE;
           }

           fprintf(fh, "\n--%s\nContent-Type: application/pgp-signature\n\n", boundary);

           snprintf(options, sizeof(options), (G->PGPVersion == 5) ? "-ab %s +batchmode=1 +force" : "-sab %s +bat +f", tf2->Filename);
           if(C->MyPGPID[0] != '\0')
           {
             strlcat(options, " -u ", sizeof(options));
             strlcat(options, C->MyPGPID, sizeof(options));
           }

           if(PGPCommand((G->PGPVersion == 5) ? "pgps" : "pgp", options, 0) == 0)
             success = TRUE;
         }
         break;

         case SEC_ENCRYPT:
         {
           char options[SIZE_LARGE];

           fprintf(fh, "Content-type: multipart/encrypted; boundary=\"%s\"; protocol=\"application/pgp-encrypted\"\n\n%s\n--%s\n", boundary, MIMEwarn, boundary);
           fprintf(fh, "Content-Type: application/pgp-encrypted\n\nVersion: 1\n\n%s\n--%s\nContent-Type: application/octet-stream\n\n", PGPwarn, boundary);

           snprintf(options, sizeof(options), (G->PGPVersion == 5) ? "-a %s %s +batchmode=1 +force" : "-ea %s %s +bat +f", tf2->Filename, ids);
           if(PGPCommand((G->PGPVersion == 5) ? "pgpe" : "pgp", options, 0) == 0)
             success = TRUE;
         }
         break;

         case SEC_BOTH:
         {
           char options[SIZE_LARGE];

           fprintf(fh, "Content-type: multipart/encrypted; boundary=\"%s\"; protocol=\"application/pgp-encrypted\"\n\n%s\n--%s\n", boundary, MIMEwarn, boundary);
           fprintf(fh, "Content-Type: application/pgp-encrypted\n\nVersion: 1\n\n%s\n--%s\nContent-Type: application/octet-stream\n\n", PGPwarn, boundary);

           snprintf(options, sizeof(options), (G->PGPVersion == 5) ? "-a %s %s +batchmode=1 +force -s" : "-sea %s %s +bat +f", tf2->Filename, ids);

           if(C->MyPGPID[0] != '\0')
           {
             strlcat(options, " -u ", sizeof(options));
             strlcat(options, C->MyPGPID, sizeof(options));
           }

           if(PGPCommand((G->PGPVersion == 5) ? "pgpe" : "pgp", options, 0) == 0)
             success = TRUE;
         }
         break;

         default:
           // nothing
         break;
      }

      if(success == TRUE)
      {
        if(EncodePart(fh, &pgppart) == FALSE)
        {
          CloseTempFile(tf2);

          RETURN(FALSE);
          return FALSE;
        }
      }
    }
    CloseTempFile(tf2);
  }

  if(pgpfile[0] != '\0')
    DeleteFile(pgpfile);

  fprintf(fh, "\n--%s--\n\n", boundary);
  FreeStrBuf(ids);
  PGPClearPassPhrase(!success);

  RETURN(success);
  return success;
}

///
/// WR_ComposeMulti
//  Assembles a multipart message
static BOOL WR_ComposeMulti(FILE *fh, const struct Compose *comp, const char *boundary)
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

    if(EncodePart(fh, p) == FALSE)
    {
      RETURN(FALSE);
      return FALSE;
    }
  }

  fprintf(fh, "\n--%s--\n\n", boundary);

  RETURN(TRUE);
  return TRUE;
}

///
/// WriteOutMessage (rec)
//  Outputs header and body of a new message
BOOL WriteOutMessage(struct Compose *comp)
{
  BOOL success;
  struct TempFile *tf=NULL;
  FILE *fh = comp->FH;
  struct WritePart *firstpart = comp->FirstPart;
  char boundary[SIZE_DEFAULT];
  char options[SIZE_DEFAULT];
  char *rcptto;

  ENTER();

  if(comp->Mode == NEW_BOUNCE)
  {
    if(comp->DelSend == TRUE)
      EmitHeader(fh, "X-YAM-Options", "delsent");

    success = WR_Bounce(fh, comp);

    RETURN(success);
    return success;
  }
  else if(comp->Mode == NEW_SAVEDEC)
  {
    if(WR_SaveDec(fh, comp) == FALSE)
    {
      RETURN(FALSE);
      return FALSE;
    }
    else
      goto mimebody;
  }

  if(firstpart == NULL)
  {
    RETURN(FALSE);
    return FALSE;
  }

  /* encrypted multipart message requested? */
  if(firstpart->Next != NULL && comp->Security > SEC_NONE  && comp->Security <= SEC_BOTH)
  {
    struct Compose tcomp;
    FILE *tfh;

    if((tf = OpenTempFile(NULL)) != NULL && (tfh = fopen(tf->Filename, "w")) != NULL)
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

      if(WriteOutMessage(&tcomp) == TRUE)    /* recurse! */
      {
        struct WritePart *tpart = comp->FirstPart; /* save parts list so we're able to recover from a calloc() error */

        /* replace with single new part */
        if((comp->FirstPart = (struct WritePart *)calloc(1,sizeof(struct WritePart))) != NULL)
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

  *options = '\0';
  if(comp->DelSend) strlcat(options, ",delsent", sizeof(options));
  if(comp->Security) snprintf(&options[strlen(options)], sizeof(options)-strlen(options), ",%s", SecCodes[comp->Security]);
  if(comp->Signature) snprintf(&options[strlen(options)], sizeof(options)-strlen(options), ",sigfile%d", comp->Signature-1);
  if(*options) EmitHeader(fh, "X-YAM-Options", &options[1]);

  if(comp->From != NULL)
    EmitRcptHeader(fh, "From", comp->From);
  else
  {
    char address[SIZE_LARGE];
    EmitRcptHeader(fh, "From", BuildAddress(address, sizeof(address), C->EmailAddress, C->RealName));
  }

  if(comp->ReplyTo) EmitRcptHeader(fh, "Reply-To", comp->ReplyTo);
  if(comp->MailTo) EmitRcptHeader(fh, "To", comp->Security == 4 ? C->ReMailer : comp->MailTo);
  if(comp->MailCC) EmitRcptHeader(fh, "CC", comp->MailCC);
  if(comp->MailBCC) EmitRcptHeader(fh, "BCC", comp->MailBCC);
  EmitHeader(fh, "Date", GetDateTime());

  // output the Message-ID, In-Reply-To and References message headers
  EmitHeader(fh, "Message-ID", NewMessageID());
  if(comp->inReplyToMsgID != NULL)
    EmitHeader(fh, "In-Reply-To", comp->inReplyToMsgID);
  if(comp->references != NULL)
    EmitHeader(fh, "References", comp->references);

  rcptto = comp->ReplyTo ? comp->ReplyTo : (comp->From ? comp->From : C->EmailAddress);
  if(comp->RequestMDN) EmitRcptHeader(fh, "Disposition-Notification-To", rcptto);
  if(comp->Importance) EmitHeader(fh, "Importance", comp->Importance == 1 ? "High" : "Low");
  fprintf(fh, "User-Agent: %s\n", yamuseragent);
  if(comp->UserInfo) WR_WriteUserInfo(fh, comp->From);
  if(*C->Organization) EmitHeader(fh, "Organization", C->Organization);
  if(*comp->Subject) EmitHeader(fh, "Subject", comp->Subject);
  if(comp->ExtHeader) WR_EmitExtHeader(fh, comp);

mimebody:

  fputs("MIME-Version: 1.0\n", fh); // RFC 2049 requires that

  strlcpy(boundary, NewBoundaryID(), sizeof(boundary));

  if(comp->GenerateMDN == TRUE)
  {
    success = WR_ComposeReport(fh, comp, boundary);
  }
  else if(comp->Security > SEC_NONE && comp->Security <= SEC_BOTH)
  {
    success = WR_ComposePGP(fh, comp, boundary);
  }
  else if(firstpart->Next != NULL)
  {
    success = WR_ComposeMulti(fh, comp, boundary);
  }
  else
  {
    WriteContentTypeAndEncoding(fh, firstpart);
    if(comp->Security == SEC_SENDANON && comp->OldSecurity != SEC_SENDANON)
      WR_Anonymize(fh, comp->MailTo);

    fputs("\n", fh);

    success = EncodePart(fh, firstpart);
  }

  CloseTempFile(tf);

  RETURN(success);
  return success;
}

///
/// WR_AutoSaveFile
//  Returns filename of the auto-save file
char *WR_AutoSaveFile(const int winnr, char *dest, const size_t length)
{
  ENTER();

  if(dest != NULL)
  {
    AddPath(dest, G->MA_MailDir, ".autosave", length);
    strlcat(dest, itoa(winnr), length);
    strlcat(dest, ".txt", length);
  }

  RETURN(dest);
  return dest;
}
///

/*** GUI ***/
/// CreateWriteWindow()
// Function that creates a new WriteWindow object and returns
// the referencing WriteMailData structure which was created
// during that process - or NULL if an error occurred.
struct WriteMailData *CreateWriteWindow(void)
{
  Object *newWriteWindow;

  ENTER();

  D(DBF_GUI, "Creating new Write Window.");

  // if we end up here we create a new WriteWindowObject
  newWriteWindow = WriteWindowObject, End;
  if(newWriteWindow)
  {
    // get the WriteMailData and check that it is the same like created
    struct WriteMailData *wmData = (struct WriteMailData *)xget(newWriteWindow, MUIA_WriteWindow_WriteMailData);

    if(wmData != NULL && wmData->window == newWriteWindow)
    {
      D(DBF_GUI, "Write window created: 0x%08lx", wmData);

      RETURN(wmData);
      return wmData;
    }

    DoMethod(G->App, OM_REMMEMBER, newWriteWindow);
    MUI_DisposeObject(newWriteWindow);
  }

  E(DBF_GUI, "ERROR occurred during write window creation!");

  RETURN(NULL);
  return NULL;
}

///

/*** WriteMailData ***/
/// CleanupWriteMailData()
// cleans/deletes all data of a WriteMailData structure
BOOL CleanupWriteMailData(struct WriteMailData *wmData)
{
  ENTER();

  SHOWVALUE(DBF_MAIL, wmData);
  ASSERT(wmData != NULL);

  // stop any pending file notification.
  if(wmData->fileNotifyActive == TRUE)
  {
    if(wmData->notifyRequest != NULL)
      EndNotify(wmData->notifyRequest);

    wmData->fileNotifyActive = FALSE;
  }

  // free the notify resources
  if(wmData->notifyRequest != NULL)
  {
    if(wmData->notifyRequest->nr_stuff.nr_Msg.nr_Port != NULL)
      FreeSysObject(ASOT_PORT, wmData->notifyRequest->nr_stuff.nr_Msg.nr_Port);

    #if defined(__amigaos4__)
    FreeDosObject(DOS_NOTIFYREQUEST, wmData->notifyRequest);
    #else
    FreeVecPooled(G->SharedMemPool, wmData->notifyRequest);
    #endif

    wmData->notifyRequest = NULL;
  }

  // delete the temp file
  DeleteFile(wmData->filename);

  // cleanup the reference mail list
  if(wmData->refMailList != NULL)
  {
    DeleteMailList(wmData->refMailList);
    wmData->refMailList = NULL;
  }

  wmData->refMail = NULL;

  // cleanup the In-Reply-To / References stuff
  if(wmData->inReplyToMsgID != NULL)
  {
    FreeStrBuf(wmData->inReplyToMsgID);
    wmData->inReplyToMsgID = NULL;
  }

  if(wmData->references != NULL)
  {
    FreeStrBuf(wmData->references);
    wmData->references = NULL;
  }

  // clean up the write window now
  if(wmData->window != NULL)
  {
    // make sure the window is really closed
    D(DBF_GUI, "make sure the write window is closed");
    nnset(wmData->window, MUIA_Window_Open, FALSE);

    D(DBF_GUI, "cleaning up write window");
    DoMethod(G->App, OM_REMMEMBER, wmData->window);
    MUI_DisposeObject(wmData->window);

    wmData->window = NULL;
  }

  // Remove the writeWindowNode and free it afterwards
  Remove((struct Node *)wmData);
  free(wmData);

  RETURN(TRUE);
  return TRUE;
}

///
/// SetWriteMailDataMailRef()
// sets the refMail and refMailList mail references to
// the one specified
BOOL SetWriteMailDataMailRef(const struct Mail *search, const struct Mail *newRef)
{
  BOOL result = FALSE;

  ENTER();

  if(IsListEmpty((struct List *)&G->writeMailDataList) == FALSE)
  {
    // search through our WriteMailDataList
    struct MinNode *curNode;

    for(curNode = G->writeMailDataList.mlh_Head; curNode->mln_Succ; curNode = curNode->mln_Succ)
    {
      struct WriteMailData *wmData = (struct WriteMailData *)curNode;

      if(wmData->refMail == search)
      {
        wmData->refMail = (struct Mail *)newRef;
        result = TRUE;
      }

      if(wmData->refMailList != NULL && IsMailListEmpty(wmData->refMailList) == FALSE)
      {
        struct MailNode *mnode;

        LockMailListShared(wmData->refMailList);

        ForEachMailNode(wmData->refMailList, mnode)
        {
          if(mnode->mail == search)
          {
            mnode->mail = (struct Mail *)newRef;
            result = TRUE;
          }
        }

        UnlockMailList(wmData->refMailList);
      }
    }
  }

  RETURN(result);
  return result;
}

///

/*** GUI ***/
/// WR_NewBounce
//  Creates a bounce window
/*
static struct WR_ClassData *WR_NewBounce(int winnum)
{
  struct WR_ClassData *data;

  ENTER();

  if((data = calloc(1, sizeof(struct WR_ClassData))) != NULL)
  {
    data->GUI.WI = WindowObject,
       MUIA_Window_Title, tr(MSG_WR_BounceWT),
       MUIA_HelpNode, "WR_W",
       MUIA_Window_ID, MAKE_ID('W','R','I','B'),
       WindowContents, VGroup,
          Child, ColGroup(2),
             Child, Label2(tr(MSG_WR_BounceTo)),
             Child, MakeAddressField(&data->GUI.ST_TO, tr(MSG_WR_BounceTo), MSG_HELP_WR_ST_TO, ABM_TO, winnum, AFF_ALLOW_MULTI|AFF_EXTERNAL_SHORTCUTS),
          End,
          Child, ColGroup(4),
             Child, data->GUI.BT_SEND   = MakeButton(tr(MSG_WR_SENDNOW)),
             Child, data->GUI.BT_QUEUE  = MakeButton(tr(MSG_WR_SENDLATER)),
             Child, data->GUI.BT_HOLD   = MakeButton(tr(MSG_WR_HOLD)),
             Child, data->GUI.BT_CANCEL = MakeButton(tr(MSG_WR_CANCEL)),
          End,
       End,
    End;
    if(data->GUI.WI != NULL)
    {
      DoMethod(G->App, OM_ADDMEMBER, data->GUI.WI);
      WR_SharedSetup(data, winnum);
    }
    else
    {
      free(data);
      data = NULL;
    }
  }

  RETURN(data);
  return data;
}
*/
///
