/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2011 YAM Open Source Team

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
#include <string.h>

#include <clib/alib_protos.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/muimaster.h>
#include <proto/utility.h>

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
#include "YAM_utilities.h"

#include "mui/ClassesExtra.h"
#include "mui/WriteWindow.h"
#include "mime/rfc2231.h"
#include "mime/rfc2047.h"
#include "mime/base64.h"
#include "mime/qprintable.h"
#include "mime/uucode.h"

#include "FileInfo.h"
#include "FolderList.h"
#include "Locale.h"
#include "MailList.h"
#include "MailServers.h"
#include "MUIObjects.h"
#include "Requesters.h"

#include "Debug.h"

/* local structures */
struct ExpandTextData
{
  const char *     OS_Name;      // Realname of original sender
  const char *     OS_Address;   // Address of original sender
  const char *     OM_Subject;   // Subject of original message
  struct DateStamp OM_Date;      // Date of original message
  int              OM_TimeZone;  // Timezone of original message
  const char *     OM_MessageID; // Message-ID of original message
  const char *     R_Name;       // Realname of person we reply/forward to
  const char *     R_Address;    // Address of person we reply/forward to
  const char *     HeaderFile;   // filename of temporary file which contain mail header
};

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
static void NewMessageID(char *idbuf, const size_t idbufSize)
{
  unsigned int seconds;
  struct DateStamp ds;
  struct MailServerNode *msn;

  ENTER();

#warning FIXME: support for multiple SMTP servers missing
  msn = GetMailServer(&C->mailServerList, MST_SMTP, 0);

  // lets calculate the seconds
  DateStamp(&ds);
  seconds = ds.ds_Days * 24 * 60 * 60 + ds.ds_Minute * 60; // seconds since 1-Jan-78

  // Here we try to generate a unique MessageID.
  // We try to be as much conform to the Recommandations for generating
  // unique Message IDs as we can: http://www.jwz.org/doc/mid.html
  snprintf(idbuf, idbufSize, "<%x%x.%x@%s>", seconds, (unsigned int)ds.ds_Tick, (unsigned int)rand(), msn->hostname);

  LEAVE();
}

///
/// NewBoundaryID
//  Creates a unique id, used for the MIME boundaries
static void NewBoundaryID(char *idbuf, const size_t idbufSize)
{
  ENTER();

  // Generate a unique Boundary ID which conforms to RFC 2045 and includes
  // a "=_" sequence to make it safe for quoted printable encoded parts
  snprintf(idbuf, idbufSize, "--=_BOUNDARY.%lx%x.%02x", (IPTR)FindTask(NULL), (unsigned int)rand(), (unsigned int)GetUniqueID() & 0xff);

  LEAVE();
}

///
/// NewMIMEpart
//  Initializes a new message part
struct WritePart *NewMIMEpart(struct WriteMailData *wmData)
{
  struct WritePart *p;

  ENTER();

  if((p = calloc(1, sizeof(*p))) != NULL)
  {
    p->ContentType = "text/plain";
    p->EncType = ENC_7BIT;

    if(wmData != NULL)
    {
      p->Filename = wmData->filename;
      p->charset = wmData->charset;
    }
    else
      p->charset = G->writeCharset;
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
    if(p->IsTemp == TRUE)
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
    fprintf(fh, "\n"
                "\t%s=\"%s\"",
                param, s);
  }
  else
  {
    size_t len = strlen(s);

    // there seem to be no "violating" characters in the string and
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
      size_t l = offset;

      // start our search
      while(len > 0)
      {
        if(*e == ' ')
          last_space = e;

        // check if we need a newline and
        // if so we go and write out the last
        // stuff including a newline.
        if(l >= 75 && last_space != NULL)
        {
          fwrite(p, last_space-p, 1, fh);

          if(len > 1)
            fwrite("\n ", 2, 1, fh);

          p = last_space+1;
          l = e-p;
          last_space = NULL;
        }

        l++;
        e++;
        len--;
      }

      if(l > 0)
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

      if((next = MyStrChr(part, ',')) != NULL)
        *next++ = '\0';

      HeaderFputs(fh, Trim(part), NULL, 0);

      part = next;
      if(part != NULL)
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
  if(body != NULL)
    EmitRcptField(fh, body);
  fputc('\n', fh);

  LEAVE();
}

///
/// EncodingName
// return the literal name for the given encoding
const char *EncodingName(const enum Encoding enc)
{
  const char *encName;

  ENTER();

  switch(enc)
  {
    case ENC_B64:
    {
      encName = "base64";
    }
    break;

    case ENC_QP:
    {
      encName = "quoted-printable";
    }
    break;

    case ENC_UUE:
    {
      encName = "x-uue";
    }
    break;

    case ENC_8BIT:
    {
      encName = "8bit";
    }
    break;

    case ENC_BIN:
    {
      encName = "binary";
    }
    break;

    default:
    case ENC_7BIT:
    {
      encName = "7bit";
    }
    break;
  }

  RETURN(encName);
  return encName;
}

///
/// WriteContentTypeAndEncoding
//  Outputs content type header including parameters
void WriteContentTypeAndEncoding(FILE *fh, const struct WritePart *part)
{
  BOOL isPrintable = FALSE;

  ENTER();

  // identify if the contenttype is printable or not
  if(strnicmp(part->ContentType, "text", 4) == 0 ||
     strnicmp(part->ContentType, "message", 7) == 0)
  {
    isPrintable = TRUE;
  }

  // output the "Content-Type:
  fprintf(fh, "Content-Type: %s", part->ContentType);
  if(part->EncType != ENC_7BIT && isPrintable == TRUE)
    fprintf(fh, "; charset=%s", strippedCharsetName(part->charset));

  // output the "name" and Content-Disposition as well
  // as the "filename" parameter to the mail
  if(part->Name != NULL && part->Name[0] != '\0')
  {
    fputc(';', fh);
    HeaderFputs(fh, part->Name, "name", 0);

    // output the Content-Disposition (RFC 2183)
    if(isPrintable == TRUE)
      fprintf(fh, "\n"
                  "Content-Disposition: inline");
    else
      fprintf(fh, "\n"
                  "Content-Disposition: attachment");

    // add the filename parameter to the Content-Disposition
    fputc(';', fh);
    HeaderFputs(fh, FilePart(part->Name), "filename", 0);
  }
  fputc('\n', fh);

  // output the Content-Transfer-Encoding
  if(part->EncType != ENC_7BIT)
    fprintf(fh, "Content-Transfer-Encoding: %s\n", EncodingName(part->EncType));

  // output the Content-Description if appropriate
  if(part->Description != NULL && part->Description[0] != '\0')
    EmitHeader(fh, "Content-Description", part->Description);

  LEAVE();
}

///
/// WR_WriteUserInfo
//  Outputs X-SenderInfo header line
static void WR_WriteUserInfo(FILE *fh, const char *from)
{
  struct ABEntry *ab = NULL;
  struct Person pers = { "", "" };

  ENTER();

  // Now we extract the real email from the address string
  if(from[0] != '\0')
    ExtractAddress(from, &pers);

  if(pers.Address[0] != '\0' && AB_SearchEntry(pers.Address, ASM_ADDRESS|ASM_USER, &ab) > 0)
  {
    if(ab->Type != AET_USER)
      ab = NULL;
    else if(ab->Homepage[0] == '\0' && ab->Phone[0] == '\0' && ab->Street[0] == '\0' && ab->City[0] == '\0' && ab->Country[0] == '\0' && ab->BirthDay == 0)
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
      if(ab->Homepage[0] != '\0')
      {
        fputc(';', fh);
        HeaderFputs(fh, ab->Homepage, "homepage", 0);
      }
      if(ab->Street[0] != '\0')
      {
        fputc(';', fh);
        HeaderFputs(fh, ab->Street, "street", 0);
      }
      if(ab->City[0] != '\0')
      {
        fputc(';', fh);
        HeaderFputs(fh, ab->City, "city", 0);
      }
      if(ab->Country[0] != '\0')
      {
        fputc(';', fh);
        HeaderFputs(fh, ab->Country, "country", 0);
      }
      if(ab->Phone[0] != '\0')
      {
        fputc(';', fh);
        HeaderFputs(fh, ab->Phone, "phone", 0);
      }
      if(ab->BirthDay != 0)
        fprintf(fh, ";\n"
                    "\tdob=%ld",
                    ab->BirthDay);
    }
    fputc('\n', fh);
  }

  LEAVE();
}

///
/// EncodePart
//  Encodes a message part
BOOL EncodePart(FILE *ofh, const struct WritePart *part)
{
  BOOL result = FALSE;
  FILE *ifh;

  ENTER();

  if((ifh = fopen(part->Filename, "r")) != NULL)
  {
    setvbuf(ifh, NULL, _IOFBF, SIZE_FILEBUF);

    switch(part->EncType)
    {
      case ENC_B64:
      {
        BOOL convLF = FALSE;

        // let us first check if we need to convert single LF to
        // CRLF to be somewhat portable.
        if(strnicmp(part->ContentType, "text", 4) == 0 ||
           strnicmp(part->ContentType, "message", 7) == 0 ||
           strnicmp(part->ContentType, "multipart", 9) == 0)
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

        fprintf(ofh, "``\n"
                     "end\n"
                     "size %d\n",
                     (int)size);
      }
      break;

      default:
      {
        D(DBF_MIME, "doing raw copy...");

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
/// WR_GetPGPId
//  Gets PGP key id for a person
static char *WR_GetPGPId(const struct Person *pe)
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
static char *WR_GetPGPIds(const char *source, char *ids)
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
static BOOL WR_Bounce(FILE *fh, const struct Compose *comp)
{
  BOOL result = FALSE;

  ENTER();

  if(comp->refMail != NULL)
  {
    char mailfile[SIZE_PATHFILE];
    FILE *oldfh;

    GetMailFile(mailfile, sizeof(mailfile), NULL, comp->refMail);
    if((oldfh = fopen(mailfile, "r")) != NULL)
    {
      char address[SIZE_LARGE];
      char msgID[SIZE_MSGID];

      setvbuf(oldfh, NULL, _IOFBF, SIZE_FILEBUF);

      // now we add the "Resent-#?" type headers which are defined
      // by RFC2822 section 3.6.6. The RFC defined that these headers
      // should be added to the top of a message
      EmitHeader(fh, "Resent-From", BuildAddress(address, sizeof(address), C->EmailAddress, C->RealName));
      EmitHeader(fh, "Resent-Date", GetDateTime());
      if(comp->MailTo != NULL)
        EmitRcptHeader(fh, "Resent-To", comp->MailTo);
      NewMessageID(msgID, sizeof(msgID));
      EmitHeader(fh, "Resent-Message-ID", msgID);

      // now we copy the rest of the message
      // directly from the file handlers
      result = CopyFile(NULL, fh, NULL, oldfh);

      fclose(oldfh);
    }
  }

  RETURN(result);
  return result;
}

///
/// WR_SaveDec
//  Creates decrypted copy of a PGP encrypted message
static BOOL WR_SaveDec(FILE *fh, const struct Compose *comp)
{
  BOOL result = FALSE;

  ENTER();

  if(comp->refMail != NULL)
  {
    char mailfile[SIZE_PATHFILE];
    char unpFile[SIZE_PATHFILE];
    BOOL xpkPacked = FALSE;
    FILE *oldfh;

    GetMailFile(mailfile, sizeof(mailfile), NULL, comp->refMail);

    // we need to analyze if the folder we are reading this mail from
    // is encrypted or compressed and then first unpacking it to a temporary file
    if(isXPKFolder(comp->refMail->Folder))
    {
      // so, this mail seems to be packed, so we need to unpack it to a temporary file
      if(StartUnpack(mailfile, unpFile, comp->refMail->Folder) != NULL &&
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

    if((oldfh = fopen(xpkPacked ? unpFile : mailfile, "r")) != NULL)
    {
      BOOL infield = FALSE;
      char *buf = NULL;
      size_t buflen = 0;

      setvbuf(oldfh, NULL, _IOFBF, SIZE_FILEBUF);

      while(getline(&buf, &buflen, oldfh) > 0)
      {
        if(buf[0] == '\n')
        {
          fprintf(fh, "X-YAM-Decrypted: PGP; %s\n", GetDateTime());
          break;
        }

        if(!isspace(buf[0]))
        {
          infield = strnicmp(buf, "content-type:", 13) == 0 ||
                    strnicmp(buf, "content-transfer-encoding", 25) == 0 ||
                    strnicmp(buf, "mime-version:", 13) == 0;
        }

        if(infield == FALSE)
          fputs(buf, fh);
      }

      fclose(oldfh);
      result = TRUE;

      free(buf);
    }

    // if we temporary unpacked the file we delete it now
    if(xpkPacked == TRUE)
      DeleteFile(unpFile);
  }

  RETURN(result);
  return result;
}

///
/// WR_EmitExtHeader
//  Outputs special X-YAM-Header lines to remember user-defined headers
static void WR_EmitExtHeader(FILE *fh, const struct Compose *comp)
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

  fprintf(fh, "Content-type: multipart/report; report-type=disposition-notification; boundary=\"%s\"\n"
              "\n",
              boundary);

  for(p = comp->FirstPart; p; p = p->Next)
  {
    fprintf(fh, "\n"
                "--%s\n",
                boundary);
    WriteContentTypeAndEncoding(fh, p);
    fputs("\n", fh);

    if(EncodePart(fh, p) == FALSE)
    {
      RETURN(FALSE);
      return FALSE;
    }
  }
  fprintf(fh, "\n"
              "--%s--\n"
              "\n",
              boundary);

  RETURN(TRUE);
  return TRUE;
}

///
/// WR_ComposePGP
//  Creates a signed and/or encrypted PGP/MIME message
static BOOL WR_ComposePGP(FILE *fh, const struct Compose *comp, char *boundary)
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
    if(comp->MailTo != NULL)
      ids = WR_GetPGPIds(comp->MailTo, ids);
    if(comp->MailCC != NULL)
      ids = WR_GetPGPIds(comp->MailCC, ids);
    if(comp->MailBCC != NULL)
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
         goto out;
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

           fprintf(fh, "Content-type: multipart/signed; boundary=\"%s\"; micalg=pgp-md5; protocol=\"application/pgp-signature\"\n"
                       "\n"
                       "%s\n"
                       "--%s\n",
                       boundary,
                       MIMEwarn,
                       boundary);
           WriteContentTypeAndEncoding(fh, firstpart);
           fputc('\n', fh);

           if(EncodePart(fh, firstpart) == FALSE)
           {
             CloseTempFile(tf2);
             goto out;
           }

           fprintf(fh, "\n"
                       "--%s\n"
                       "Content-Type: application/pgp-signature\n"
                       "\n",
                       boundary);

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

           fprintf(fh, "Content-type: multipart/encrypted; boundary=\"%s\"; protocol=\"application/pgp-encrypted\"\n"
                       "\n"
                       "%s\n"
                       "--%s\n",
                       boundary,
                       MIMEwarn,
                       boundary);
           fprintf(fh, "Content-Type: application/pgp-encrypted\n\nVersion: 1\n"
                       "\n"
                       "%s\n"
                       "--%s\n"
                       "Content-Type: application/octet-stream\n"
                       "\n",
                       PGPwarn,
                       boundary);

           snprintf(options, sizeof(options), (G->PGPVersion == 5) ? "-a %s %s +batchmode=1 +force" : "-ea %s %s +bat +f", tf2->Filename, ids);
           if(PGPCommand((G->PGPVersion == 5) ? "pgpe" : "pgp", options, 0) == 0)
             success = TRUE;
         }
         break;

         case SEC_BOTH:
         {
           char options[SIZE_LARGE];

           fprintf(fh, "Content-type: multipart/encrypted; boundary=\"%s\"; protocol=\"application/pgp-encrypted\"\n"
                       "\n"
                       "%s\n"
                       "--%s\n",
                       boundary,
                       MIMEwarn,
                       boundary);
           fprintf(fh, "Content-Type: application/pgp-encrypted\n"
                       "\n"
                       "Version: 1\n"
                       "\n"
                       "%s\n"
                       "--%s\n"
                       "Content-Type: application/octet-stream\n"
                       "\n",
                       PGPwarn,
                       boundary);

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
          goto out;
        }
      }
    }
    CloseTempFile(tf2);
  }

  if(pgpfile[0] != '\0')
    DeleteFile(pgpfile);

  fprintf(fh, "\n"
              "--%s--\n"
              "\n",
              boundary);

out:
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

  fprintf(fh, "Content-type: multipart/mixed; boundary=\"%s\"\n"
              "\n",
              boundary);
  fputs(MIMEwarn, fh);

  for(p = comp->FirstPart; p; p = p->Next)
  {
    fprintf(fh, "\n"
                "--%s\n",
                boundary);

    WriteContentTypeAndEncoding(fh, p);

    fputs("\n", fh);

    if(EncodePart(fh, p) == FALSE)
    {
      RETURN(FALSE);
      return FALSE;
    }
  }

  fprintf(fh, "\n"
              "--%s--\n"
              "\n",
              boundary);

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
  char buf[SIZE_DEFAULT];
  char msgID[SIZE_MSGID];

  ENTER();

  if(comp->Mode == NMM_BOUNCE)
  {
    if(comp->DelSend == TRUE)
      EmitHeader(fh, "X-YAM-Options", "delsent");

    success = WR_Bounce(fh, comp);

    RETURN(success);
    return success;
  }
  else if(comp->Mode == NMM_SAVEDEC)
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

      memcpy(&tcomp, comp, sizeof(tcomp));   // clone struct Compose
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
        // save parts list so we're able to recover from a calloc() error
        struct WritePart *tpart = comp->FirstPart;

        // replace with single new part
        if((comp->FirstPart = (struct WritePart *)calloc(1,sizeof(struct WritePart))) != NULL)
        {
          // reuse encoding
          comp->FirstPart->EncType = tpart->EncType;
          // free old parts list
          FreePartsList(tpart);
          // the only part is an email message
          comp->FirstPart->ContentType = "message/rfc822";
          // set filename to tempfile
          comp->FirstPart->Filename = tf->Filename;
          // only use sig in enclosed mail
          comp->Signature = 0;
        }
        else
        {
          // no errormsg here - the window probably won't open anyway...
          DisplayBeep(NULL);
          // just restore old parts list and switch off security
          comp->FirstPart = tpart;
          comp->Security = 0;
          // we'll most likely get more errors further down :(
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

  buf[0] = '\0';
  if(comp->DelSend == TRUE)
    strlcat(buf, ",delsent", sizeof(buf));
  if(comp->Security != 0)
    snprintf(&buf[strlen(buf)], sizeof(buf)-strlen(buf), ",%s", SecCodes[comp->Security]);
  if(comp->Signature != 0)
    snprintf(&buf[strlen(buf)], sizeof(buf)-strlen(buf), ",sigfile%d", comp->Signature-1);
  if(buf[0] != '\0')
    EmitHeader(fh, "X-YAM-Options", &buf[1]);

  if(comp->From != NULL)
    EmitRcptHeader(fh, "From", comp->From);
  else
  {
    char address[SIZE_LARGE];

    EmitRcptHeader(fh, "From", BuildAddress(address, sizeof(address), C->EmailAddress, C->RealName));
  }

  if(comp->ReplyTo != NULL)
    EmitRcptHeader(fh, "Reply-To", comp->ReplyTo);
  if(comp->MailTo != NULL)
    EmitRcptHeader(fh, "To", comp->MailTo);
  if(comp->MailCC != NULL)
    EmitRcptHeader(fh, "CC", comp->MailCC);
  if(comp->MailBCC != NULL)
    EmitRcptHeader(fh, "BCC", comp->MailBCC);
  EmitHeader(fh, "Date", GetDateTime());

  // output the Message-ID, In-Reply-To and References message headers
  NewMessageID(msgID, sizeof(msgID));
  EmitHeader(fh, "Message-ID", msgID);
  if(comp->inReplyToMsgID != NULL)
    EmitHeader(fh, "In-Reply-To", comp->inReplyToMsgID);
  if(comp->references != NULL)
    EmitHeader(fh, "References", comp->references);

  if(comp->RequestMDN == TRUE)
  {
    if(comp->ReplyTo != NULL)
      EmitRcptHeader(fh, "Disposition-Notification-To", comp->ReplyTo);
    else if(comp->From != NULL)
      EmitRcptHeader(fh, "Disposition-Notification-To", comp->From);
    else
      EmitRcptHeader(fh, "Disposition-Notification-To", C->EmailAddress);
  }
  if(comp->Importance != 0)
    EmitHeader(fh, "Importance", comp->Importance == 1 ? "High" : "Low");

  fprintf(fh, "User-Agent: %s\n", yamuseragent);

  if(comp->UserInfo == TRUE)
    WR_WriteUserInfo(fh, comp->From);

  // if the PGP key ID is set we go and output the OpenPGP header
  // field which is defined at http://josefsson.org/openpgp-header/
  if(C->MyPGPID[0] != '\0')
  {
    char *p = strchr(C->MyPGPID, 'x');
    if(p == NULL)
      p = C->MyPGPID;
    else
      p++;

    fprintf(fh, "OpenPGP: id=%s", p);

    if(C->PGPURL[0] != '\0')
    {
      fputc(';', fh);
      HeaderFputs(fh, C->PGPURL, "url", 0);
    }

    fputc('\n', fh);
  }

  if(C->Organization[0] != '\0')
    EmitHeader(fh, "Organization", C->Organization);

  if(comp->Subject[0] != '\0')
    EmitHeader(fh, "Subject", comp->Subject);

  if(comp->ExtHeader != NULL)
    WR_EmitExtHeader(fh, comp);

mimebody:
  // RFC 2049 requires this
  fputs("MIME-Version: 1.0\n", fh);

  NewBoundaryID(buf, sizeof(buf));

  if(comp->GenerateMDN == TRUE)
  {
    success = WR_ComposeReport(fh, comp, buf);
  }
  else if(comp->Security > SEC_NONE && comp->Security <= SEC_BOTH)
  {
    success = WR_ComposePGP(fh, comp, buf);
  }
  else if(firstpart->Next != NULL)
  {
    success = WR_ComposeMulti(fh, comp, buf);
  }
  else
  {
    WriteContentTypeAndEncoding(fh, firstpart);
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
    snprintf(dest, length, "%s%02d.txt", dest, winnr);
  }

  RETURN(dest);
  return dest;
}

///
/// AppendRcpt()
//  Appends a recipient address to a string
static char *AppendRcpt(char *sbuf, const struct Person *pe, const BOOL excludeme)
{
  ENTER();

  if(pe != NULL)
  {
    D(DBF_MAIL, "add recipient for person named '%s', address '%s'", SafeStr(pe->RealName), SafeStr(pe->Address));

    // Make sure that the person has at least either name or address and
    // that these are non-empty strings. Otherwise we will add invalid
    // recipients like '@domain' without any real name and user name.
    if((pe->Address != NULL && pe->Address[0] != '\0') ||
       (pe->RealName != NULL && pe->RealName[0] != '\0'))
    {
      char address[SIZE_LARGE];
      char *ins;
      BOOL skip = FALSE;

      if(strchr(pe->Address, '@') != NULL)
      {
        ins = BuildAddress(address, sizeof(address), pe->Address, pe->RealName);
      }
      else
      {
        char addr[SIZE_ADDRESS];
        char *p = strchr(C->EmailAddress, '@');

        snprintf(addr, sizeof(addr), "%s%s", pe->Address, p ? p : "");
        ins = BuildAddress(address, sizeof(address), addr, pe->RealName);
      }

      if(ins != NULL)
      {
        // exclude the given person if it is ourself
        if(excludeme == TRUE && stricmp(pe->Address, C->EmailAddress) == 0)
          skip = TRUE;

        // if the string already contains this person then skip it
        if(stristr(sbuf, ins) != NULL)
          skip = TRUE;

        if(skip == FALSE)
        {
          D(DBF_MAIL, "adding recipient '%s'", ins);

          // lets prepend a ", " sequence in case sbuf
          // is not empty
          if(*sbuf != '\0')
            sbuf = StrBufCat(sbuf, ", ");

          sbuf = StrBufCat(sbuf, ins);
        }
      }
    }
  }

  RETURN(sbuf);
  return sbuf;
}

///

/*** ExpandText ***/
/// ExpandText()
//  Replaces variables with values
static char *ExpandText(const char *src, struct ExpandTextData *etd)
{
  char buf[SIZE_ADDRESS];
  char *p;
  char *p2;
  char *dst = AllocStrBuf(SIZE_DEFAULT);

  ENTER();

  for(; *src; src++)
  {
    if(*src == '\\')
    {
      src++;
      switch(*src)
      {
        case '\\':
          dst = StrBufCat(dst, "\\");
        break;

        case 'n':
          dst = StrBufCat(dst, "\n");
        break;
      }
    }
    else if(*src == '%' && etd != NULL)
    {
      src++;
      switch(*src)
      {
        case 'n':
          dst = StrBufCat(dst, etd->OS_Name);
        break;

        case 'f':
        {
          strlcpy(buf, etd->OS_Name, sizeof(buf));

          if((p = strchr(buf, ',')) != NULL)
            p = Trim(++p);
          else
          {
            for(p = buf; *p && *p != ' '; p++);

            *p = 0;
            p = buf;
          }
          dst = StrBufCat(dst, p);
        }
        break;

        case 's':
          dst = StrBufCat(dst, etd->OM_Subject);
        break;

        case 'e':
          dst = StrBufCat(dst, etd->OS_Address);
        break;

        case 'd':
        {
          char datstr[64];

          DateStamp2String(datstr, sizeof(datstr), &etd->OM_Date, DSS_DATE, TZC_NONE);
          dst = StrBufCat(dst, datstr);
        }
        break;

        case 't':
        {
          char datstr[64];

          DateStamp2String(datstr, sizeof(datstr), &etd->OM_Date, DSS_TIME, TZC_NONE);
          dst = StrBufCat(dst, datstr);
        }
        break;

        case 'z':
        {
          char tzone[6];

          int convertedTimeZone = (etd->OM_TimeZone/60)*100 + (etd->OM_TimeZone%60);
          snprintf(tzone, sizeof(tzone), "%+05d", convertedTimeZone);
          dst = StrBufCat(dst, tzone);
        }
        break;

        case 'w':
        {
          char datstr[64];

          DateStamp2String(datstr, sizeof(datstr), &etd->OM_Date, DSS_WEEKDAY, TZC_NONE);
          dst = StrBufCat(dst, datstr);
        }
        break;

        case 'c':
        {
          char datstr[64];

          DateStamp2RFCString(datstr, sizeof(datstr), &etd->OM_Date, etd->OM_TimeZone, FALSE);
          dst = StrBufCat(dst, datstr);
        }
        break;

        case 'm':
          dst = StrBufCat(dst, etd->OM_MessageID);
        break;

        case 'r':
          dst = StrBufCat(dst, etd->R_Name);
        break;

        case 'v':
        {
          strlcpy(buf, etd->R_Name, sizeof(buf));
          if((p = strchr(buf, ',')) != NULL)
            p = Trim(++p);
          else
          {
            for(p = buf; *p && *p != ' '; p++);

            *p = '\0';
            p = buf;
          }
          dst = StrBufCat(dst, p);
        }
        break;

        case 'a':
          dst = StrBufCat(dst, etd->R_Address);
        break;

        case 'i':
        {
          strlcpy(buf, etd->OS_Name, sizeof(buf));

          for(p = p2 = &buf[1]; *p; p++)
          {
            if(*p == ' ' && p[1] && p[1] != ' ')
              *p2++ = *++p;
          }
          *p2 = '\0';
          dst = StrBufCat(dst, buf);
        }
        break;

        case 'j':
        {
          strlcpy(buf, etd->OS_Name, sizeof(buf));

          for(p2 = &buf[1], p = &buf[strlen(buf)-1]; p > p2; p--)
          {
            if(p[-1] == ' ')
            {
              *p2++ = *p;
              break;
            }
          }
          *p2 = '\0';
          dst = StrBufCat(dst, buf);
        }
        break;

        case 'h':
        {
          if((p = FileToBuffer(etd->HeaderFile)))
          {
            dst = StrBufCat(dst, p);
            free(p);
          }
        }
        break;
      }
    }
    else
    {
       char chr[2];

       chr[0] = *src;
       chr[1] = '\0';
       dst = StrBufCat(dst, chr);
    }
  }

  RETURN(dst);
  return dst;
}

///
/// InsertIntroText()
//  Inserts a phrase into the message text
static void InsertIntroText(FILE *fh, const char *text, struct ExpandTextData *etd)
{
  ENTER();

  if(text[0] != '\0')
  {
    char *sbuf;

    if((sbuf = ExpandText(text, etd)) != NULL)
    {
      fprintf(fh, "%s\n", sbuf);
      FreeStrBuf(sbuf);
    }
  }

  LEAVE();
}

///
/// SetupExpandTextData()
//  Creates quote string by replacing variables with values
static void SetupExpandTextData(struct ExpandTextData *etd, const struct Mail *mail)
{
  ENTER();

  etd->OS_Name     = (mail != NULL) ? ((mail->From.RealName[0] != '\0') ? mail->From.RealName : mail->From.Address) : "";
  etd->OS_Address  = (mail != NULL) ? mail->From.Address : "";
  etd->OM_Subject  = (mail != NULL) ? mail->Subject : "";
  etd->OM_TimeZone = (mail != NULL) ? mail->tzone : C->TimeZone;
  etd->R_Name      = "";
  etd->R_Address   = "";

  // we have to copy the datestamp and eventually convert it
  // according to the timezone
  if(mail != NULL)
  {
    // the mail time is in UTC, so we have to convert it to the
    // actual time of the mail as we don't do any conversion
    // later on
    memcpy(&etd->OM_Date, &mail->Date, sizeof(struct DateStamp));

    if(mail->tzone != 0)
    {
      struct DateStamp *date = &etd->OM_Date;

      date->ds_Minute += mail->tzone;

      // we need to check the datestamp variable that it is still in it's borders
      // after adjustment
      while(date->ds_Minute < 0)
      {
        date->ds_Minute += 1440;
        date->ds_Days--;
      }
      while(date->ds_Minute >= 1440)
      {
        date->ds_Minute -= 1440;
        date->ds_Days++;
      }
    }
  }
  else
    memcpy(&etd->OM_Date, &G->StartDate, sizeof(struct DateStamp));

  LEAVE();
}

///

/*** GUI ***/
/// CreateWriteWindow()
// Function that creates a new WriteWindow object and returns
// the referencing WriteMailData structure which was created
// during that process - or NULL if an error occurred.
struct WriteMailData *CreateWriteWindow(const enum NewMailMode mailMode, const BOOL quietMode)
{
  Object *newWriteWindow;

  ENTER();

  D(DBF_GUI, "Creating new Write Window.");

  // if we end up here we create a new WriteWindowObject
  newWriteWindow = WriteWindowObject,
                     MUIA_WriteWindow_Mode,  mailMode,
                     MUIA_WriteWindow_Quiet, quietMode,
                   End;

  if(newWriteWindow != NULL)
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
/// NewWriteMailWindow()
//  Creates a new, empty write message
struct WriteMailData *NewWriteMailWindow(struct Mail *mail, const int flags)
{
  BOOL quiet = hasQuietFlag(flags);
  struct Folder *folder = G->currentFolder;
  struct WriteMailData *wmData = NULL;

  ENTER();

  // First check if the basic configuration is okay, then open write window */
  if(folder != NULL && CO_IsValid() == TRUE &&
     (wmData = CreateWriteWindow(NMM_NEW, quiet)) != NULL)
  {
    FILE *out;

    if((out = fopen(wmData->filename, "w")) != NULL)
    {
      setvbuf(out, NULL, _IOFBF, SIZE_FILEBUF);

      wmData->refMail = mail;

      if(wmData->refMail != NULL)
      {
        char address[SIZE_LARGE];

        // check whether the old mail contains a ReplyTo: address
        // or not. And if so we prefer that one instead of using the
        // To: adresses
        if(mail->ReplyTo.Address[0] != '\0')
        {
          char *addr = BuildAddress(address, sizeof(address), mail->ReplyTo.Address, mail->ReplyTo.RealName);

          if(addr != NULL)
          {
            struct ExtendedMail *email;

            if(isMultiReplyToMail(mail) &&
               (email = MA_ExamineMail(mail->Folder, mail->MailFile, TRUE)) != NULL)
            {
              int i;
              char *sbuf;

              // add all "ReplyTo:" recipients of the mail
              sbuf = StrBufCpy(NULL, addr);
              for(i=0; i < email->NoSReplyTo; i++)
                sbuf = AppendRcpt(sbuf, &email->SReplyTo[i], FALSE);

              set(wmData->window, MUIA_WriteWindow_To, sbuf);

              FreeStrBuf(sbuf);
              MA_FreeEMailStruct(email);
            }
            else
              set(wmData->window, MUIA_WriteWindow_To, addr);
          }
        }
        else
        {
          char *addr = BuildAddress(address, sizeof(address), mail->From.Address, mail->From.RealName);

          if(addr != NULL)
          {
            struct ExtendedMail *email;

            if(isMultiSenderMail(mail) &&
              (email = MA_ExamineMail(mail->Folder, mail->MailFile, TRUE)) != NULL)
            {
              char *sbuf;
              int i;

              // add all "From:" recipients of the mail
              sbuf = StrBufCpy(NULL, addr);
              for(i=0; i < email->NoSFrom; i++)
                sbuf = AppendRcpt(sbuf, &email->SFrom[i], FALSE);

              set(wmData->window, MUIA_WriteWindow_To, sbuf);

              FreeStrBuf(sbuf);
              MA_FreeEMailStruct(email);
            }
            else
              set(wmData->window, MUIA_WriteWindow_To, addr);
          }
        }
      }
      else if(folder->MLSupport == TRUE)
      {
        if(folder->MLAddress[0] != '\0')
          set(wmData->window, MUIA_WriteWindow_To, folder->MLAddress);

        if(folder->MLFromAddress[0] != '\0')
          set(wmData->window, MUIA_WriteWindow_From, folder->MLFromAddress);

        if(folder->MLReplyToAddress[0] != '\0')
          set(wmData->window, MUIA_WriteWindow_ReplyTo, folder->MLReplyToAddress);
      }

      if(folder->WriteIntro[0] != '\0')
        InsertIntroText(out, folder->WriteIntro, NULL);
      else
        InsertIntroText(out, C->NewIntro, NULL);

      if(folder->WriteGreetings[0] != '\0')
        InsertIntroText(out, folder->WriteGreetings, NULL);
      else
        InsertIntroText(out, C->Greetings, NULL);

      // close the output file handle
      fclose(out);

      // add a signature to the mail depending on the selected signature for this list
      DoMethod(wmData->window, MUIM_WriteWindow_AddSignature, folder->MLSupport ? folder->MLSignature: -1);

      // update the message text
      DoMethod(wmData->window, MUIM_WriteWindow_ReloadText, FALSE);

      // make sure the window is opened
      if(quiet == FALSE)
        SafeOpenWindow(wmData->window);

      if(C->LaunchAlways == TRUE && quiet == FALSE)
        DoMethod(wmData->window, MUIM_WriteWindow_LaunchEditor);
    }
    else
    {
      CleanupWriteMailData(wmData);
      wmData = NULL;
    }
  }

  RETURN(wmData);
  return wmData;
}

///
/// NewEditMailWindow()
//  Edits a message in a new write window
struct WriteMailData *NewEditMailWindow(struct Mail *mail, const int flags)
{
  BOOL quiet = hasQuietFlag(flags);
  struct Folder *folder;
  struct WriteMailData *wmData = NULL;

  ENTER();

  // check the parameters
  if(mail == NULL || (folder = mail->Folder) == NULL)
  {
    RETURN(wmData);
    return wmData;
  }

  // check if the mail in question resists in the outgoing
  // folder
  if(isOutgoingFolder(folder))
  {
    // search through our WriteMailDataList
    struct Node *curNode;

    IterateList(&G->writeMailDataList, curNode)
    {
      struct WriteMailData *lwmData = (struct WriteMailData *)curNode;

      if(lwmData->window != NULL && lwmData->refMail == mail)
      {
        DoMethod(lwmData->window, MUIM_Window_ToFront);

        RETURN(lwmData);
        return lwmData;
      }
    }
  }

  // check if necessary settings fror writing are OK and open new window
  if(CO_IsValid() == TRUE &&
     (wmData = CreateWriteWindow(isOutgoingFolder(folder) ? NMM_EDIT : NMM_EDITASNEW, quiet)) != NULL)
  {
    FILE *out;

    if((out = fopen(wmData->filename, "w")) != NULL)
    {
      char *sbuf = NULL;
      struct ReadMailData *rmData;
      struct ExtendedMail *email;

      setvbuf(out, NULL, _IOFBF, SIZE_FILEBUF);

      wmData->refMail = mail;

      if((email = MA_ExamineMail(folder, mail->MailFile, TRUE)) == NULL)
      {
        char mailfile[SIZE_PATHFILE];

        GetMailFile(mailfile, sizeof(mailfile), NULL, mail);
        ER_NewError(tr(MSG_ER_CantOpenFile), mailfile);
        fclose(out);
        CleanupWriteMailData(wmData);

        RETURN(NULL);
        return NULL;
      }

      if((rmData = AllocPrivateRMData(mail, PM_ALL)) != NULL)
      {
        char *cmsg;

        if((cmsg = RE_ReadInMessage(rmData, RIM_EDIT)) != NULL)
        {
          int msglen = strlen(cmsg);

          // we check whether cmsg contains any text and if so we
          // write out the whole text to our temporary file.
          if(msglen == 0 || fwrite(cmsg, msglen, 1, out) == 1)
          {
            int i;
            char address[SIZE_LARGE];

            // free our temp text now
            free(cmsg);

            // set the In-Reply-To / References message header references, if they exist
            if(email->inReplyToMsgID != NULL)
              wmData->inReplyToMsgID = StrBufCpy(NULL, email->inReplyToMsgID);

            if(email->references != NULL)
              wmData->references = StrBufCpy(NULL, email->references);

            // set the subject gadget
            set(wmData->window, MUIA_WriteWindow_Subject, mail->Subject);

            // in case this is a EDITASNEW action we have to make sure
            // to add the From: and ReplyTo: address of the user of
            // YAM instead of filling in the data of the mail we
            // are trying to edit.
            if(wmData->mode == NMM_EDITASNEW)
            {
              if(folder->MLSupport == TRUE)
              {
                if(folder->MLFromAddress[0] != '\0')
                  set(wmData->window, MUIA_WriteWindow_From, folder->MLFromAddress);

                if(folder->MLReplyToAddress[0] != '\0')
                  set(wmData->window, MUIA_WriteWindow_ReplyTo, folder->MLReplyToAddress);
              }
            }
            else
            {
              // use all From:/ReplyTo: from the original mail
              // instead.

              // add all From: senders
              sbuf = StrBufCpy(sbuf, BuildAddress(address, sizeof(address), mail->From.Address, mail->From.RealName));
              for(i=0; i < email->NoSFrom; i++)
                sbuf = AppendRcpt(sbuf, &email->SFrom[i], FALSE);

              set(wmData->window, MUIA_WriteWindow_From, sbuf);

              // add all ReplyTo: recipients
              sbuf = StrBufCpy(sbuf, BuildAddress(address, sizeof(address), mail->ReplyTo.Address, mail->ReplyTo.RealName));
              for(i=0; i < email->NoSReplyTo; i++)
                sbuf = AppendRcpt(sbuf, &email->SReplyTo[i], FALSE);

              set(wmData->window, MUIA_WriteWindow_ReplyTo, sbuf);
            }

            // add all "To:" recipients of the mail
            sbuf = StrBufCpy(sbuf, BuildAddress(address, sizeof(address), mail->To.Address, mail->To.RealName));
            for(i=0; i < email->NoSTo; i++)
              sbuf = AppendRcpt(sbuf, &email->STo[i], FALSE);

            set(wmData->window, MUIA_WriteWindow_To, sbuf);

            // add all "CC:" recipients of the mail
            sbuf[0] = '\0';
            for(i=0; i < email->NoCC; i++)
            {
              sbuf = AppendRcpt(sbuf, &email->CC[i], FALSE);
            }
            set(wmData->window, MUIA_WriteWindow_Cc, sbuf);

            // add all "BCC:" recipients of the mail
            sbuf[0] = '\0';
            for(i=0; i < email->NoBCC; i++)
            {
              sbuf = AppendRcpt(sbuf, &email->BCC[i], FALSE);
            }
            set(wmData->window, MUIA_WriteWindow_BCC, sbuf);

            // free our temporary buffer
            FreeStrBuf(sbuf);

            if(email->extraHeaders != NULL)
              set(wmData->window, MUIA_WriteWindow_ExtHeaders, email->extraHeaders);

            xset(wmData->window, MUIA_WriteWindow_DelSend,    email->DelSend,
                                 MUIA_WriteWindow_MDN,        isSendMDNMail(mail),
                                 MUIA_WriteWindow_AddInfo,    isSenderInfoMail(mail),
                                 MUIA_WriteWindow_Importance, getImportanceLevel(mail) == IMP_HIGH ? 0 : getImportanceLevel(mail)+1,
                                 MUIA_WriteWindow_Signature,  email->Signature,
                                 MUIA_WriteWindow_Security,   email->Security);

            // let us safe the security state
            wmData->oldSecurity = email->Security;

            // setup the write window from an existing readmailData structure
            DoMethod(wmData->window, MUIM_WriteWindow_SetupFromOldMail, rmData);
          }
          else
          {
            E(DBF_MAIL, "Error while writing cmsg to out FH");

            // an error occurred while trying to write the text to out
            free(cmsg);
            FreePrivateRMData(rmData);
            fclose(out);
            MA_FreeEMailStruct(email);

            CleanupWriteMailData(wmData);

            RETURN(NULL);
            return NULL;
          }
        }

        FreePrivateRMData(rmData);
      }

      fclose(out);
      MA_FreeEMailStruct(email);

      // update the message text
      DoMethod(wmData->window, MUIM_WriteWindow_ReloadText, FALSE);

      // make sure the window is opened
      if(quiet == FALSE)
        SafeOpenWindow(wmData->window);

      sbuf = (STRPTR)xget(wmData->window, MUIA_WriteWindow_To);
      set(wmData->window, MUIA_WriteWindow_ActiveObject, *sbuf ? MUIV_WriteWindow_ActiveObject_TextEditor :
                                                                 MUIV_WriteWindow_ActiveObject_To);

      if(C->LaunchAlways == TRUE && quiet == FALSE)
        DoMethod(wmData->window, MUIM_WriteWindow_LaunchEditor);
    }
    else
    {
      CleanupWriteMailData(wmData);
      wmData = NULL;
    }
  }

  RETURN(wmData);
  return wmData;
}

///
/// NewForwardMailWindow()
//  Forwards a list of messages
struct WriteMailData *NewForwardMailWindow(struct MailList *mlist, const int flags)
{
  BOOL quiet = hasQuietFlag(flags);
  struct WriteMailData *wmData = NULL;

  ENTER();

  // check if necessary settings fror writing are OK and open new window
  if(CO_IsValid() == TRUE &&
     (wmData = CreateWriteWindow(NMM_FORWARD, quiet)) != NULL)
  {
    FILE *out;

    if((out = fopen(wmData->filename, "w")) != NULL)
    {
      int signature = -1;
      enum ForwardMode fwdMode = C->ForwardMode;
      char *rsub = AllocStrBuf(SIZE_SUBJECT);
      struct MailNode *mnode;

      // if the user wants to have the alternative
      // forward mode we go and select it here
      if(hasAsAttachmentFwdModeFlag(flags))
      {
        fwdMode = FWM_ATTACH;
      }
      else if(hasInlinedFwdModeFlag(flags))
      {
        fwdMode = FWM_INLINE;
      }
      else if(hasAltFwdModeFlag(flags))
      {
        switch(fwdMode)
        {
          case FWM_ATTACH:
            fwdMode = FWM_INLINE;
          break;

          case FWM_INLINE:
            fwdMode = FWM_ATTACH;
          break;
        }
      }

      // set the output filestream buffer size
      setvbuf(out, NULL, _IOFBF, SIZE_FILEBUF);

      // set the write mode
      wmData->refMailList = CloneMailList(mlist);

      // sort the mail list by date
      SortMailList(mlist, MA_CompareByDate);

      // insert the intro text
      InsertIntroText(out, C->NewIntro, NULL);

      // iterate through all the mail in the
      // mail list and build up the forward text
      ForEachMailNode(mlist, mnode)
      {
        struct ExtendedMail *email;
        struct ExpandTextData etd;
        struct Mail *mail = mnode->mail;

        if(signature == -1 && mail->Folder != NULL)
        {
          if(mail->Folder->MLSupport == TRUE)
            signature = mail->Folder->MLSignature;
        }

        if((email = MA_ExamineMail(mail->Folder, mail->MailFile, TRUE)) == NULL)
        {
          char mailfile[SIZE_PATHFILE];

          GetMailFile(mailfile, sizeof(mailfile), NULL, mail);
          ER_NewError(tr(MSG_ER_CantOpenFile), mailfile);
          fclose(out);
          FreeStrBuf(rsub);

          CleanupWriteMailData(wmData);

          RETURN(NULL);
          return NULL;
        }

        SetupExpandTextData(&etd, &email->Mail);
        etd.OM_MessageID = email->messageID;
        etd.R_Name = mail->To.RealName[0] != '\0' ? mail->To.RealName : mail->To.Address;
        etd.R_Address = mail->To.Address;

        // we create a generic subject line for the forward
        // action so that a forwarded mail will have a [Fwd: XXX] kinda
        // subject line instead of the original.
        if(mail->Subject != '\0')
        {
          char buffer[SIZE_LARGE];

          snprintf(buffer, sizeof(buffer), "[Fwd: %s]", mail->Subject);
          if(strstr(rsub, buffer) == NULL)
          {
            if(rsub[0] != '\0')
              rsub = StrBufCat(rsub, "; ");

            rsub = StrBufCat(rsub, buffer);
          }
        }

        // depending on the selected forward mode we either
        // forward the email as inlined text or by simply putting
        // the original message as an attachment to the new one.
        switch(fwdMode)
        {
          // let simply add the original mail as an attachment
          case FWM_ATTACH:
          {
            char filename[SIZE_PATHFILE];
            struct Attach attach;

            memset(&attach, 0, sizeof(struct Attach));

            GetMailFile(filename, sizeof(filename), NULL, mail);
            if(StartUnpack(filename, attach.FilePath, mail->Folder) != NULL)
            {
              strlcpy(attach.Description, mail->Subject, sizeof(attach.Description));
              strlcpy(attach.ContentType, "message/rfc822", sizeof(attach.ContentType));
              attach.Size = mail->Size;
              attach.IsMIME = TRUE;

              // add the attachment to our attachment listview
              DoMethod(wmData->window, MUIM_WriteWindow_InsertAttachment, &attach);
            }
            else
              E(DBF_MAIL, "unpacking of file '%s' failed!", filename);
          }
          break;

          // inline the message text of our original mail to
          // our forward message
          case FWM_INLINE:
          {
            struct ReadMailData *rmData;

            // we allocate some private readmaildata object so that
            // we can silently parse the mail which we want to
            // forward.
            if((rmData = AllocPrivateRMData(mail, PM_ALL)) != NULL)
            {
              char *cmsg;

              etd.HeaderFile = rmData->firstPart->Filename;
              InsertIntroText(out, C->ForwardIntro, &etd);

              // read in the message text to cmsg.
              if((cmsg = RE_ReadInMessage(rmData, RIM_FORWARD)) != NULL)
              {
                // output the readin message text immediately to
                // our out filehandle
                fputs(cmsg, out);
                free(cmsg);

                InsertIntroText(out, C->ForwardFinish, &etd);

                // if the mail we are forwarding has an attachment
                // we go and attach them to our forwarded mail as well.
                if(hasNoAttachFlag(flags) == FALSE)
                  DoMethod(wmData->window, MUIM_WriteWindow_SetupFromOldMail, rmData);
              }

              FreePrivateRMData(rmData);
            }
          }
          break;
        }

        // free out temporary extended mail structure again.
        MA_FreeEMailStruct(email);
      }

      // add some footer with greatings.
      InsertIntroText(out, C->Greetings, NULL);
      fclose(out);

      // add a signature to the mail depending on the selected signature for this list
      DoMethod(wmData->window, MUIM_WriteWindow_AddSignature, signature);

      // set the composed subject text
      set(wmData->window, MUIA_WriteWindow_Subject, rsub);
      FreeStrBuf(rsub);

      // update the message text
      DoMethod(wmData->window, MUIM_WriteWindow_ReloadText, FALSE);

      // make sure the window is opened
      if(quiet == FALSE)
        SafeOpenWindow(wmData->window);

      // set the active object of the window
      set(wmData->window, MUIA_WriteWindow_ActiveObject, MUIV_WriteWindow_ActiveObject_To);

      if(C->LaunchAlways == TRUE && quiet == FALSE)
        DoMethod(wmData->window, MUIM_WriteWindow_LaunchEditor);
    }
    else
    {
      CleanupWriteMailData(wmData);
      wmData = NULL;
    }
  }

  RETURN(wmData);
  return wmData;
}

///
/// NewReplyMailWindow()
//  Creates a reply to a list of messages
struct WriteMailData *NewReplyMailWindow(struct MailList *mlist, const int flags, const char *replytxt)
{
  BOOL quiet = hasQuietFlag(flags);
  struct WriteMailData *wmData = NULL;

  ENTER();

  // check if necessary settings fror writing are OK and open new window
  if(CO_IsValid() == TRUE &&
     (wmData = CreateWriteWindow(NMM_REPLY, quiet)) != NULL)
  {
    FILE *out;

    if((out = fopen(wmData->filename, "w")) != NULL)
    {
      int j;
      int repmode = 1;
      int signature = -1;
      BOOL altpat = FALSE;
      char *domain = NULL;
      char *mlistad = NULL;
      char *rfrom = NULL;
      char *rrepto = NULL;
      char *rto = AllocStrBuf(SIZE_ADDRESS);
      char *rcc = AllocStrBuf(SIZE_ADDRESS);
      char *rsub = AllocStrBuf(SIZE_SUBJECT);
      char buffer[SIZE_LARGE];
      struct ExpandTextData etd;
      BOOL mlIntro = FALSE;
      struct MailNode *mnode;

      setvbuf(out, NULL, _IOFBF, SIZE_FILEBUF);

      // make sure the write window know of the
      // operation and knows which mails to process
      wmData->refMailList = CloneMailList(mlist);

      // make sure we sort the mlist according to
      // the mail date
      SortMailList(mlist, MA_CompareByDate);

      // Now we iterate through all selected mails
      j = 0;
      ForEachMailNode(mlist, mnode)
      {
        int k;
        struct Mail *mail = mnode->mail;
        struct Folder *folder = mail->Folder;
        struct ExtendedMail *email;
        struct Person pe;
        BOOL foundMLFolder = FALSE;
        BOOL foundSentFolder = FALSE;

        if((email = MA_ExamineMail(folder, mail->MailFile, TRUE)) == NULL)
        {
          char mailfile[SIZE_PATHFILE];

          GetMailFile(mailfile, sizeof(mailfile), NULL, mail);
          ER_NewError(tr(MSG_ER_CantOpenFile), mailfile);
          fclose(out);
          CleanupWriteMailData(wmData);
          FreeStrBuf(rto);
          FreeStrBuf(rcc);
          FreeStrBuf(rsub);

          RETURN(NULL);
          return NULL;
        }

        // make sure we setup the quote string
        // correctly.
        SetupExpandTextData(&etd, &email->Mail);
        etd.OM_MessageID = email->messageID;

        // If the mail which we are going to reply to already has a subject,
        // we are going to add a "Re:" to it.
        if(mail->Subject[0] != '\0')
        {
          if(j > 0)
          {
            // if the subject contains brackets then these need to be removed first,
            // else the strstr() call below will not find the "reduced" subject
            if(mail->Subject[0] == '[' && strchr(mail->Subject, ']') != NULL)
            {
              // copy the stripped subject
              strlcpy(buffer, MA_GetRealSubject(mail->Subject), sizeof(buffer));
            }
            else
            {
              // copy the subject as-is
              strlcpy(buffer, mail->Subject, sizeof(buffer));
            }
          }
          else
          {
            // copy the first subject stripped, but prepend the usual "Re:"
            snprintf(buffer, sizeof(buffer), "Re: %s", MA_GetRealSubject(mail->Subject));
          }

          // try to find following subjects in the yet created reply subject
          if(strstr(rsub, buffer) == NULL)
          {
            if(rsub[0] != '\0')
              rsub = StrBufCat(rsub, "; ");

            rsub = StrBufCat(rsub, buffer);
          }
        }

        // in case we are replying to a single message we also have to
        // save the messageID of the email we are replying to
        if(wmData->inReplyToMsgID != NULL)
          wmData->inReplyToMsgID = StrBufCat(wmData->inReplyToMsgID, " ");

        if(email->messageID != NULL)
          wmData->inReplyToMsgID = StrBufCat(wmData->inReplyToMsgID, email->messageID);

        // in addition, we check for "References:" message header stuff
        if(wmData->references != NULL)
          wmData->references = StrBufCat(wmData->references, " ");

        if(email->references != NULL)
          wmData->references = StrBufCat(wmData->references, email->references);
        else
        {
          // check if this email contains inReplyToMsgID data and if so we
          // create a new references header entry
          if(email->inReplyToMsgID != NULL)
            wmData->references = StrBufCat(wmData->references, email->inReplyToMsgID);
        }

        // Now we analyse the folder of the selected mail and if it
        // is a mailing list we have to do some special operation
        if(folder != NULL)
        {
          // if the mail we are going to reply resists in the incoming folder
          // we have to check all other folders first.
          if(isIncomingFolder(folder))
          {
            struct FolderNode *fnode;

            LockFolderListShared(G->folders);

            // walk through all our folders
            // and check if it matches a pattern
            ForEachFolderNode(G->folders, fnode)
            {
              if(fnode->folder != NULL && fnode->folder->MLSupport == TRUE && fnode->folder->MLPattern[0] != '\0')
              {
                char *pattern = fnode->folder->MLPattern;

                if(MatchNoCase(mail->To.Address, pattern) == FALSE &&
                   MatchNoCase(mail->To.RealName, pattern) == FALSE)
                {
                  for(k=0; k < email->NoSTo; k++)
                  {
                    if(MatchNoCase(email->STo[k].Address, pattern) ||
                       MatchNoCase(email->STo[k].RealName, pattern))
                    {
                      foundMLFolder = TRUE;
                      break;
                    }
                  }
                }
                else
                  foundMLFolder = TRUE;

                if(foundMLFolder == TRUE)
                {
                  mlistad = fnode->folder->MLAddress[0] != '\0' ? fnode->folder->MLAddress : NULL;
                  folder = fnode->folder;

                  if(folder->MLFromAddress[0] != '\0')
                    rfrom  = folder->MLFromAddress;

                  if(folder->MLReplyToAddress[0] != '\0')
                    rrepto = folder->MLReplyToAddress;

                  break;
                }
              }
            }

            UnlockFolderList(G->folders);
          }
          else if(isSentMailFolder(folder))
          {
            foundSentFolder = TRUE;
          }
          else if(folder->MLSupport == TRUE && folder->MLPattern[0] != '\0')
          {
            if(MatchNoCase(mail->To.Address, folder->MLPattern) == FALSE &&
               MatchNoCase(mail->To.RealName, folder->MLPattern) == FALSE)
            {
              for(k=0; k < email->NoSTo; k++)
              {
                if(MatchNoCase(email->STo[k].Address, folder->MLPattern) ||
                   MatchNoCase(email->STo[k].RealName, folder->MLPattern))
                {
                  foundMLFolder = TRUE;
                  break;
                }
              }
            }
            else
              foundMLFolder = TRUE;

            if(foundMLFolder == TRUE)
            {
              mlistad = folder->MLAddress[0] != '\0' ? folder->MLAddress : NULL;

              if(folder->MLFromAddress[0] != '\0')
                rfrom  = folder->MLFromAddress;

              if(folder->MLReplyToAddress[0] != '\0')
                rrepto = folder->MLReplyToAddress;
            }
          }
        }

        // If this mail is a standard multi-recipient mail and the user hasn't pressed SHIFT
        // or ALT we going to ask him to which recipient he want to send the mail to.
        if(isMultiRCPTMail(mail) && hasPrivateFlag(flags) == FALSE && hasMListFlag(flags) == FALSE)
        {
          // ask the user and in case he want to abort, quit this
          // function immediately.
          if((repmode = MUI_Request(G->App, G->MA->GUI.WI, 0, NULL, tr(MSG_MA_ReplyReqOpt), tr(MSG_MA_ReplyReq))) == 0)
          {
            MA_FreeEMailStruct(email);
            fclose(out);
            CleanupWriteMailData(wmData);
            FreeStrBuf(rto);
            FreeStrBuf(rcc);
            FreeStrBuf(rsub);

            RETURN(NULL);
            return NULL;
          }
        }

        // now we should know how the user wants to
        // reply to the mail. The possible reply modes are:
        //
        // repmode == 1 : To Sender (From:/ReplyTo:)
        // repmode == 2 : To Sender and all recipients (From:/ReplyTo:, To:, CC:)
        // repmode == 3 : To Recipients (To:, CC:)
        if(repmode == 1)
        {
          BOOL addDefault = FALSE;

          // the user wants to reply to the Sender (From:), however we
          // need to check whether he want to get asked or directly reply to
          // the wanted address.
          if(foundSentFolder == TRUE)
          {
            // the mail to which the user wants to reply is stored in a
            // sent mail folder. As such all mail should originate from ourself
            // and as such when he presses "reply" on it we send it to
            // the To: address recipient instead.
            rto = AppendRcpt(rto, &mail->To, FALSE);
            for(k=0; k < email->NoSTo; k++)
              rto = AppendRcpt(rto, &email->STo[k], FALSE);
          }
          else if(hasPrivateFlag(flags) == TRUE)
          {
            // the user seem to have pressed the SHIFT key, so
            // we are going to "just"reply to the "From:" addresses of
            // the original mail. so we add them accordingly.
            rto = AppendRcpt(rto, &mail->From, FALSE);
            for(k=0; k < email->NoSFrom; k++)
              rto = AppendRcpt(rto, &email->SFrom[k], FALSE);
          }
          else if(foundMLFolder == TRUE && mlistad != NULL)
          {
            char *p;

            if((p = strdup(mlistad)) != NULL)
            {
              char *ptr = p;

              // we found a matching folder for the mail we are going to
              // reply to, so we go and add the 'mlistad' to our To: addresses
              while(ptr != NULL && *ptr != '\0')
              {
                char *next;

                if((next = MyStrChr(ptr, ',')) != NULL)
                  *next++ = '\0';

                ExtractAddress(ptr, &pe);
                rto = AppendRcpt(rto, &pe, FALSE);

                ptr = next;
              }

              free(p);
            }
          }
          else if(C->CompareAddress == TRUE && hasMListFlag(flags) == FALSE &&
                  mail->ReplyTo.Address[0] != '\0')
          {
            BOOL askUser = FALSE;

            // now we have to check whether the ReplyTo: and From: of the original are the
            // very same or not.
            if(stricmp(mail->From.Address, mail->ReplyTo.Address) == 0)
            {
              if(email->NoSFrom == email->NoSReplyTo)
              {
                for(k=0; k < email->NoSFrom; k++)
                {
                  if(stricmp(email->SFrom[k].Address, email->SReplyTo[k].Address) != 0)
                  {
                    askUser = TRUE;
                    break;
                  }
                }
              }
              else
                askUser = TRUE;
            }
            else
              askUser = TRUE;

            // if askUser == TRUE, we go and
            // ask the user which address he wants to reply to.
            if(askUser == TRUE)
            {
              snprintf(buffer, sizeof(buffer), tr(MSG_MA_CompareReq), mail->From.Address, mail->ReplyTo.Address);

              switch(MUI_Request(G->App, G->MA->GUI.WI, 0, NULL, tr(MSG_MA_Compare3ReqOpt), buffer))
              {
                // Both (From:/ReplyTo:) address
                case 3:
                {
                  // add all From: addresses to the CC: list
                  rcc = AppendRcpt(rcc, &mail->From, FALSE);
                  for(k=0; k < email->NoSFrom; k++)
                    rcc = AppendRcpt(rcc, &email->SFrom[k], FALSE);
                }
                // continue

                // Reply-To: addresses
                case 2:
                {
                  rto = AppendRcpt(rto, &mail->ReplyTo, FALSE);
                  for(k=0; k < email->NoSReplyTo; k++)
                    rto = AppendRcpt(rto, &email->SReplyTo[k], FALSE);
                }
                break;

                // only From: addresses
                case 1:
                {
                  rto = AppendRcpt(rto, &mail->From, FALSE);
                  for(k=0; k < email->NoSFrom; k++)
                    rto = AppendRcpt(rto, &email->SFrom[k], FALSE);
                }
                break;

                // cancel operation
                case 0:
                {
                  MA_FreeEMailStruct(email);
                  fclose(out);
                  CleanupWriteMailData(wmData);
                  FreeStrBuf(rto);
                  FreeStrBuf(rcc);
                  FreeStrBuf(rsub);

                  RETURN(NULL);
                  return NULL;
                }
              }
            }
            else
              addDefault = TRUE;
          }
          else
            addDefault = TRUE;

          if(addDefault == TRUE)
          {
            // otherwise we check whether to use the ReplyTo: or From: addresses as the
            // To: adress of our reply. If a ReplyTo: exists we use that one instead
            if(mail->ReplyTo.Address[0] != '\0')
            {
              rto = AppendRcpt(rto, &mail->ReplyTo, FALSE);
              for(k=0; k < email->NoSReplyTo; k++)
                rto = AppendRcpt(rto, &email->SReplyTo[k], FALSE);
            }
            else
            {
              rto = AppendRcpt(rto, &mail->From, FALSE);
              for(k=0; k < email->NoSFrom; k++)
                rto = AppendRcpt(rto, &email->SFrom[k], FALSE);
            }
          }
        }
        else
        {
          // user wants to reply to all senders and recipients
          // so let's add them
          if(repmode == 2)
          {
            if(mail->ReplyTo.Address[0] != '\0')
            {
              rto = AppendRcpt(rto, &mail->ReplyTo, FALSE);
              for(k=0; k < email->NoSReplyTo; k++)
                rto = AppendRcpt(rto, &email->SReplyTo[k], FALSE);
            }
            else
            {
              rto = AppendRcpt(rto, &mail->From, FALSE);
              for(k=0; k < email->NoSFrom; k++)
                rto = AppendRcpt(rto, &email->SFrom[k], FALSE);
            }
          }

          // now add all original To: addresses
          rto = AppendRcpt(rto, &mail->To, TRUE);
          for(k=0; k < email->NoSTo; k++)
            rto = AppendRcpt(rto, &email->STo[k], TRUE);

          // add the CC: addresses as well
          for(k=0; k < email->NoCC; k++)
            rcc = AppendRcpt(rcc, &email->CC[k], TRUE);
        }

        // extract the first address/name from our generated
        // To: address string
        ExtractAddress(rto, &pe);
        etd.R_Name = pe.RealName;
        etd.R_Address = pe.Address;

        // extract the domain name from the To address or respective
        // the default To: mail address
        if((domain = strchr(pe.Address, '@')) == NULL)
          domain = strchr(C->EmailAddress, '@');

        if(C->AltReplyPattern[0] != '\0' && domain != NULL && MatchNoCase(domain, C->AltReplyPattern))
          altpat = TRUE;
        else
          altpat = FALSE;

        // insert a "Hello" text as the first intro text in case
        // this is our first iteration
        if(j == 0)
        {
          if(foundMLFolder == TRUE)
          {
            signature = folder->MLSignature;
            mlIntro = TRUE;
          }

          if(foundSentFolder == TRUE)
          {
            if(folder != NULL && folder->WriteIntro[0] != '\0')
              InsertIntroText(out, folder->WriteIntro, NULL);
            else
              InsertIntroText(out, C->NewIntro, NULL);
          }
          else
            InsertIntroText(out, foundMLFolder ? C->MLReplyHello : (altpat ? C->AltReplyHello : C->ReplyHello), &etd);
        }

        // if the user wants to quote the mail text of the original mail,
        // we process it right now.
        if(C->QuoteMessage == TRUE && !hasNoQuoteFlag(flags))
        {
          struct ReadMailData *rmData;

          if(j > 0)
            fputc('\n', out);

          if((rmData = AllocPrivateRMData(mail, PM_TEXTS)) != NULL)
          {
            char *cmsg;

            etd.HeaderFile = rmData->firstPart->Filename;

            // put some introduction right before the quoted text.
            InsertIntroText(out, foundMLFolder ? C->MLReplyIntro : (altpat ? C->AltReplyIntro : C->ReplyIntro), &etd);

            // if there has been another (preferred) reply text given
            // we use that one instead
            if(replytxt != NULL && replytxt[0] != '\0')
            {
              // make sure we quote the text in question.
              QuoteText(out, replytxt, strlen(replytxt), C->EdWrapMode != EWM_OFF ? C->EdWrapCol-2 : 1024);
            }
            else if((cmsg = RE_ReadInMessage(rmData, RIM_QUOTE)) != NULL)
            {
              // make sure we quote the text in question.
              QuoteText(out, cmsg, strlen(cmsg), C->EdWrapMode != EWM_OFF ? C->EdWrapCol-2 : 1024);

              free(cmsg);
            }

            FreePrivateRMData(rmData);
          }
        }

        // free out temporary extended mail structure again.
        MA_FreeEMailStruct(email);

        j++;
      }

      // now we complement the "References:" header by adding our replyto header to it
      if(wmData->inReplyToMsgID != NULL)
      {
        if(wmData->references != NULL)
          wmData->references = StrBufCat(wmData->references, " ");

        wmData->references = StrBufCat(wmData->references, wmData->inReplyToMsgID);
      }

      // now that the mail is finished, we go and output some footer message to
      // the reply text.
      InsertIntroText(out, mlIntro ? C->MLReplyBye : (altpat ? C->AltReplyBye: C->ReplyBye), &etd);

      // close the filehandle
      fclose(out);

      // add a signature to the mail depending on the selected signature for this list
      DoMethod(wmData->window, MUIM_WriteWindow_AddSignature, signature);

      // If this is a reply to a mail belonging to a mailing list,
      // set the "From:" and "Reply-To:" addresses accordingly */
      if(rfrom != NULL)
        set(wmData->window, MUIA_WriteWindow_From, rfrom);

      if(rrepto != NULL)
        set(wmData->window, MUIA_WriteWindow_ReplyTo, rrepto);

      xset(wmData->window, MUIA_WriteWindow_To, rto,
                           rto[0] != '\0' ? MUIA_WriteWindow_Cc : MUIA_WriteWindow_To, rcc,
                           MUIA_WriteWindow_Subject, rsub);

      // update the message text
      DoMethod(wmData->window, MUIM_WriteWindow_ReloadText, FALSE);

      // make sure the window is opened
      if(quiet == FALSE)
        SafeOpenWindow(wmData->window);

      // set the active object of the window
      set(wmData->window, MUIA_WriteWindow_ActiveObject, MUIV_WriteWindow_ActiveObject_TextEditor);

      if(C->LaunchAlways == TRUE && quiet == FALSE)
        DoMethod(wmData->window, MUIM_WriteWindow_LaunchEditor);

      // free our temporary buffers
      FreeStrBuf(rto);
      FreeStrBuf(rcc);
      FreeStrBuf(rsub);
    }
    else
    {
      CleanupWriteMailData(wmData);
      wmData = NULL;
    }
  }

  RETURN(wmData);
  return wmData;
}

///
/// NewBounceMailWindow()
//  Bounces a message
struct WriteMailData *NewBounceMailWindow(struct Mail *mail, const int flags)
{
  BOOL quiet = hasQuietFlag(flags);
  struct WriteMailData *wmData = NULL;

  ENTER();

  // check if necessary settings fror writing are OK and open new window
  if(CO_IsValid() == TRUE &&
     (wmData = CreateWriteWindow(NMM_BOUNCE, quiet)) != NULL)
  {
    wmData->refMail = mail;

    // make sure the window is opened
    if(quiet == FALSE)
      SafeOpenWindow(wmData->window);

    // set the active object of the window
    set(wmData->window, MUIA_WriteWindow_ActiveObject, MUIV_WriteWindow_ActiveObject_To);
  }

  RETURN(wmData);
  return wmData;
}

///

/*** WriteMailData ***/
/// AllocWriteMailData
// creates a WriteMailData structure
struct WriteMailData *AllocWriteMailData(void)
{
  struct WriteMailData *wmData;

  ENTER();

  if((wmData = AllocSysObjectTags(ASOT_NODE, ASONODE_Size, sizeof(*wmData),
                                             ASONODE_Min, TRUE,
                                             TAG_DONE)) != NULL)
  {
    memset(wmData, 0, sizeof(*wmData));
    wmData->mode = NMM_NEW;
    DateStamp(&wmData->lastFileChangeTime);
    wmData->oldSecurity = SEC_NONE;
  }

  RETURN(wmData);
  return wmData;
}

///
/// FreeWriteMailData
// free a WriteMailData structure
void FreeWriteMailData(struct WriteMailData *wmData)
{
  ENTER();

  FreeSysObject(ASOT_NODE, wmData);

  LEAVE();
}

///
/// CleanupWriteMailData()
// cleans/deletes all data of a WriteMailData structure
BOOL CleanupWriteMailData(struct WriteMailData *wmData)
{
  ENTER();

  SHOWVALUE(DBF_MAIL, wmData);
  ASSERT(wmData != NULL);

  // check if this wmData is the current active Rexx background
  // processing one and if so set the ptr to NULL to signal the rexx
  // commands that their active window was closed/disposed
  if(wmData == G->ActiveRexxWMData)
    G->ActiveRexxWMData = NULL;

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
  FreeWriteMailData(wmData);

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
  struct Node *curNode;

  ENTER();

  IterateList(&G->writeMailDataList, curNode)
  {
    struct WriteMailData *wmData = (struct WriteMailData *)curNode;

    if(wmData->refMail == search)
    {
      wmData->refMail = (struct Mail *)newRef;
      result = TRUE;
    }

    if(wmData->refMailList != NULL)
    {
      struct MailNode *mnode;

      LockMailListShared(wmData->refMailList);

      if((mnode = FindMailInList(wmData->refMailList, search)) != NULL)
      {
        mnode->mail = (struct Mail *)newRef;
        result = TRUE;
      }

      UnlockMailList(wmData->refMailList);
    }
  }

  RETURN(result);
  return result;
}

///

/*** AutoSave files ***/
/// CheckForAutoSaveFiles()
// function that checks for .autosaveXX.txt files and
// warn the user accordingly of the existance of such a backup file
void CheckForAutoSaveFiles(void)
{
  static const char *pattern = ".autosave[0-9][0-9].txt";
  char *parsedPattern;
  LONG parsedPatternSize;
  APTR context;

  ENTER();

  // we go and check whether there is any .autosaveXX.txt file in the maildir
  // directory. And if so we ask the user what he would like to do with it.
  parsedPatternSize = strlen(pattern) * 2 + 2;
  if((parsedPattern = malloc(parsedPatternSize)) != NULL)
  {
    ParsePatternNoCase(pattern, parsedPattern, parsedPatternSize);

    #if defined(__amigaos4__)
    // dos.library before 52.17 has a small bug and needs a hook for the matching process
    if((context = ObtainDirContextTags(EX_StringName,  (ULONG)G->MA_MailDir,
                                       EX_MatchString, (ULONG)parsedPattern,
                                       EX_MatchFunc,   LIB_VERSION_IS_AT_LEAST(DOSBase, 52, 17) ? NULL : &ExamineDirMatchHook,
                                       TAG_DONE)) != NULL)
    #else
    if((context = ObtainDirContextTags(EX_StringName,  (ULONG)G->MA_MailDir,
                                       EX_MatchString, (ULONG)parsedPattern,
                                       TAG_DONE)) != NULL)
    #endif
    {
      struct ExamineData *ed;
      LONG error;

      while((ed = ExamineDir(context)) != NULL)
      {
        // check that this entry is a file
        // because we don't accept any dir here
        if(EXD_IS_FILE(ed))
        {
          int answer;
          char fileName[SIZE_PATHFILE];

          D(DBF_MAIL, "found file '%s' matches autosave pattern '%s'", ed->Name, pattern);

          // pack the filename and path together so that we can reference to it
          AddPath(fileName, G->MA_MailDir, ed->Name, sizeof(fileName));

          // now that we have identified the existance of a .autosave file
          // we go and warn the user accordingly.
          answer = MUI_Request(G->App, G->MA->GUI.WI, 0, tr(MSG_MA_AUTOSAVEFOUND_TITLE),
                                                         tr(MSG_MA_AUTOSAVEFOUND_BUTTONS),
                                                         tr(MSG_MA_AUTOSAVEFOUND),
                                                         fileName);
          if(answer == 1)
          {
            // the user wants to put the autosave file on hold in the outgoing folder
            // so lets do it and delete the autosave file afterwards
            struct WriteMailData *wmData;

            if((wmData = NewWriteMailWindow(NULL, NEWF_QUIET)) != NULL)
            {
              // set some default receiver and subject, because the autosave file just contains
              // the message text
              xset(wmData->window, MUIA_WriteWindow_To, "no@receiver",
                                   MUIA_WriteWindow_Subject, "(subject)");

              // load the file in the new editor gadget and flag it as changed
              DoMethod(wmData->window, MUIM_WriteWindow_LoadText, fileName, TRUE);

              // put the new mail on hold
              DoMethod(wmData->window, MUIM_WriteWindow_ComposeMail, WRITE_HOLD);

              // we need to explicitly delete the autosave file here because
              // the delete routine in WR_NewMail() doesn't catch the correct file
              // because it only cares about the autosave file for the newly created
              // write object
              if(DeleteFile(fileName) == 0)
                AddZombieFile(fileName);
            }
          }
          else if(answer == 2)
          {
            // the user wants to open the autosave file in an own new write window,
            // so lets do it and delete the autosave file afterwards
            struct WriteMailData *wmData;

            if((wmData = NewWriteMailWindow(NULL, 0)) != NULL)
            {
              // load the file in the new editor gadget and flag it as changed
              DoMethod(wmData->window, MUIM_WriteWindow_LoadText, fileName, TRUE);

              // we delete the autosave file now
              if(DeleteFile(fileName) == 0)
                AddZombieFile(fileName);

              // then we immediately create a new autosave file
              DoMethod(wmData->window, MUIM_WriteWindow_DoAutoSave);
            }
          }
          else if(answer == 3)
          {
            // just delete the autosave file
            if(DeleteFile(fileName) == 0)
              AddZombieFile(fileName);
          }
        }
      }

      if((error = IoErr()) != ERROR_NO_MORE_ENTRIES)
        E(DBF_ALWAYS, "ExamineDir() failed, error %ld", error);

      ReleaseDirContext(context);
    }

    free(parsedPattern);
  }

  LEAVE();
}

///
