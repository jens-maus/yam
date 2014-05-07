/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2014 YAM Open Source Team

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
#include <proto/codesets.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/muimaster.h>
#include <proto/utility.h>

#include "extrasrc.h"

#include "YAM.h"
#include "YAM_utilities.h"
#include "YAM_error.h"
#include "YAM_stringsizes.h"
#include "YAM_mainFolder.h"
#include "YAM_global.h"
#include "YAM_utilities.h"

#include "mui/ClassesExtra.h"
#include "mui/WriteWindow.h"
#include "mui/YAMApplication.h"
#include "mime/rfc2231.h"
#include "mime/rfc2047.h"
#include "mime/base64.h"
#include "mime/qprintable.h"
#include "mime/uucode.h"

#include "AddressBook.h"
#include "Config.h"
#include "DynamicString.h"
#include "FileInfo.h"
#include "FolderList.h"
#include "Locale.h"
#include "MailList.h"
#include "MailServers.h"
#include "MUIObjects.h"
#include "Requesters.h"
#include "Signature.h"
#include "UserIdentity.h"

#include "Debug.h"

/* local structures */
struct ExpandTextData
{
  const char *     OS_Name;      // Realname of original sender
  const char *     OS_Address;   // Address of original sender
  const char *     OM_Subject;   // Subject of original message
  struct DateStamp OM_Date;      // Date of original message
  int              OM_gmtOffset; // GMT offset original message
  const char *     OM_tzAbbr;    // timeZone Abbreviation of original message
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

  DateStamp2RFCString(dt, sizeof(dt), NULL, G->gmtOffset, G->tzAbbr, FALSE);

  RETURN(dt);
  return dt;
}

///
/// NewMessageID
//  Creates a unique id, used for Message-ID header field
static void NewMessageID(char *idbuf, const size_t idbufSize, const struct MailServerNode *msn)
{
  ENTER();

  if(msn != NULL)
  {
    unsigned int seconds;
    struct DateStamp ds;

    // lets calculate the seconds
    DateStamp(&ds);
    seconds = ds.ds_Days * 24 * 60 * 60 + ds.ds_Minute * 60; // seconds since 1-Jan-78

    // Here we try to generate a unique MessageID.
    // We try to be as much conform to the Recommandations for generating
    // unique Message IDs as we can: http://www.jwz.org/doc/mid.html
    snprintf(idbuf, idbufSize, "<%x%x.%x@%s>", seconds, (unsigned int)ds.ds_Tick, (unsigned int)rand(), msn->hostname);
  }
  else
  {
    E(DBF_MAIL, "no SMTP server configured");
    idbuf[0] = '\0';
  }

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
    p->Size = -1;

    if(wmData != NULL)
    {
      p->Filename = wmData->filename;
      p->Codeset = wmData->codeset;
    }
    else
      p->Codeset = G->writeCodeset;
  }
  else
    E(DBF_MAIL, "couldn't create new MIME part");

  RETURN(p);
  return p;
}

///
/// FreePartsList
//  Clears message parts and deletes temporary files
void FreePartsList(struct WritePart *p, BOOL delTemp)
{
  ENTER();

  while(p != NULL)
  {
    struct WritePart *np = p->Next;

    if(p->IsTemp == TRUE && delTemp == TRUE)
      DeleteFile(p->Filename);

    free(p);

    p = np;
  }

  LEAVE();
}

///
/// HeaderFputs
// Outputs the value of a header line directly to the FILE pointer and handle
// RFC2231 complicant MIME parameter encoding, but also RFC2047 compliant
// MIME value encoding as well. As soon as "param" is set to NULL, RFC2047
// encoding will be used, otherwise RFC2231-based one.
static void HeaderFputs(FILE *fh, const char *s, const char *param, const int offset, struct codeset *codeset)
{
  BOOL doEncoding = FALSE;
  char *c = (char *)s;
  size_t sLen;
  size_t paramLen = 0;
  char *sCS = NULL;
  char *paramCS = NULL;

  ENTER();

  sLen = strlen(s);
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
    if(codeset != NULL && codeset != G->localCodeset)
    {
      // convert the strings to the desired codeset and replace the old
      // strings with them
      sCS = CodesetsConvertStr(CSA_SourceCodeset,   G->localCodeset,
                               CSA_DestCodeset,     codeset,
                               CSA_Source,          s,
                               CSA_SourceLen,       sLen,
                               CSA_MapForeignChars, C->MapForeignChars,
                               TAG_DONE);
      s = (const char *)sCS;

      if(param != NULL)
      {
        paramCS = CodesetsConvertStr(CSA_SourceCodeset,   G->localCodeset,
                                     CSA_DestCodeset,     codeset,
                                     CSA_Source,          param,
                                     CSA_SourceLen,       paramLen,
                                     CSA_MapForeignChars, C->MapForeignChars,
                                     TAG_DONE);
        param = (const char *)paramCS;
      }
    }

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

    if(codeset != NULL && codeset != G->localCodeset)
    {
      if(sCS != NULL)
        CodesetsFreeA(sCS, NULL);
      if(paramCS != NULL)
        CodesetsFreeA(paramCS, NULL);
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
    // there seem to be no "violating" characters in the string and
    // the resulting string will also be not > 78 chars in case we
    // have to encode a MIME parameter, so we go and output the source
    // string immediately
    D(DBF_MAIL, "writing plain content '%s'", s);

    // all we have to make sure now is that we don't write longer lines
    // than 78 chars or we fold them
    if(sLen >= (size_t)75-offset)
    {
      char *p = (char *)s;
      char *e = (char *)s;
      char *last_space = NULL;
      size_t l = offset;

      // start our search
      while(sLen > 0)
      {
        if(*e == ' ')
          last_space = e;

        // check if we need a newline and
        // if so we go and write out the last
        // stuff including a newline.
        if(l >= 75 && last_space != NULL)
        {
          fwrite(p, last_space-p, 1, fh);

          if(sLen > 1)
            fwrite("\n ", 2, 1, fh);

          p = last_space+1;
          l = e-p;
          last_space = NULL;
        }

        l++;
        e++;
        sLen--;
      }

      if(l > 0)
        fwrite(p, e-p, 1, fh);
    }
    else
      fwrite(s, sLen, 1, fh);
  }

  LEAVE();
}

///
/// EmitHeader
//  Outputs a complete header line
void EmitHeader(FILE *fh, const char *hdr, const char *body, struct codeset *codeset)
{
  ENTER();

  D(DBF_MAIL, "writing header '%s' with content '%s'", hdr, SafeStr(body));

  if(body != NULL)
  {
    int offset;

    offset = fprintf(fh, "%s: ", hdr);
    HeaderFputs(fh, body, NULL, offset, codeset);
    fputc('\n', fh);
  }

  LEAVE();
}

///
/// EmitRcptField
//  Outputs the value of a recipient header line, one entry per line
static void EmitRcptField(FILE *fh, const char *body, struct codeset *codeset)
{
  char *bodycpy;

  ENTER();

  D(DBF_MAIL, "writing recipient field with content '%s'", SafeStr(body));

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

      HeaderFputs(fh, Trim(part), NULL, 0, codeset);

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
static void EmitRcptHeader(FILE *fh, const char *hdr, const char *body, struct codeset *codeset)
{
  ENTER();

  if(body != NULL)
  {
    D(DBF_MAIL, "writing recipient header '%s' with content '%s'", hdr, SafeStr(body));

    fprintf(fh, "%s: ", hdr);
    EmitRcptField(fh, body, codeset);
    fputc('\n', fh);
  }
  else
    D(DBF_MAIL, "omitting recipient header '%s' because of empty body", hdr);

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
    fprintf(fh, "; charset=%s", strippedCharsetName(part->Codeset));

  // output the "name" and Content-Disposition as well
  // as the "filename" parameter to the mail
  if(IsStrEmpty(part->Name) == FALSE)
  {
    fputc(';', fh);
    HeaderFputs(fh, part->Name, "name", 0, NULL);

    // output the Content-Disposition (RFC 2183)
    if(part->IsAttachment == FALSE && isPrintable == TRUE)
      fprintf(fh, "\n"
                  "Content-Disposition: inline");
    else
      fprintf(fh, "\n"
                  "Content-Disposition: attachment");

    // add the filename parameter to the Content-Disposition
    fputc(';', fh);
    HeaderFputs(fh, FilePart(part->Name), "filename", 0, NULL);

    // add the attachment's size
    if(part->Size != -1)
    {
      fprintf(fh, ";\n"
                  "\tsize=%ld", part->Size);
    }
  }
  fputc('\n', fh);

  // output the Content-Transfer-Encoding
  if(part->EncType != ENC_7BIT)
    fprintf(fh, "Content-Transfer-Encoding: %s\n", EncodingName(part->EncType));

  // output the Content-Description if appropriate
  if(IsStrEmpty(part->Description) == FALSE)
    EmitHeader(fh, "Content-Description", part->Description, NULL);

  LEAVE();
}

///
/// WR_WriteUserInfo
//  Outputs X-SenderInfo header line
static void WR_WriteUserInfo(FILE *fh, struct Compose *comp)
{
  struct ABookNode *ab = NULL;

  ENTER();

  // Now we extract the real email from the address string
  if(SearchABook(&G->abook, comp->Identity->address, ASM_ADDRESS|ASM_USER, &ab) > 0)
  {
    if(ab->type != ABNT_USER)
      ab = NULL;
    else if(ab->Homepage[0] == '\0' && ab->Phone[0] == '\0' && ab->Street[0] == '\0' && ab->City[0] == '\0' && ab->Country[0] == '\0' && ab->Birthday == 0)
      ab = NULL;
  }

  if(ab != NULL || comp->Identity->photoURL[0] != '\0')
  {
    fputs("X-SenderInfo: 1", fh);
    if(comp->Identity->photoURL[0] != '\0')
    {
      fputc(';', fh);
      HeaderFputs(fh, comp->Identity->photoURL, "picture", 0, comp->codeset);
    }

    if(ab != NULL)
    {
      if(ab->Homepage[0] != '\0')
      {
        fputc(';', fh);
        HeaderFputs(fh, ab->Homepage, "homepage", 0, comp->codeset);
      }
      if(ab->Street[0] != '\0')
      {
        fputc(';', fh);
        HeaderFputs(fh, ab->Street, "street", 0, comp->codeset);
      }
      if(ab->City[0] != '\0')
      {
        fputc(';', fh);
        HeaderFputs(fh, ab->City, "city", 0, comp->codeset);
      }
      if(ab->Country[0] != '\0')
      {
        fputc(';', fh);
        HeaderFputs(fh, ab->Country, "country", 0, comp->codeset);
      }
      if(ab->Phone[0] != '\0')
      {
        fputc(';', fh);
        HeaderFputs(fh, ab->Phone, "phone", 0, comp->codeset);
      }
      if(ab->Birthday != 0)
        fprintf(fh, ";\n"
                    "\tdob=%ld",
                    ab->Birthday);
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
  struct ABookNode *ab = NULL;

  ENTER();

  if(SearchABook(&G->abook, pe->RealName, ASM_REALNAME|ASM_USER, &ab) == 0)
  {
    SearchABook(&G->abook, pe->Address, ASM_ADDRESS|ASM_USER, &ab);
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

    dstrcat(&ids, (G->PGPVersion == 5) ? "-r \"" : "\"");
    dstrcat(&ids, pid);
    dstrcat(&ids, "\" ");
  }

  RETURN(ids);
  return ids;
}

///
/// WR_Redirect
//  Redirect message: inserts resent-headers while keeping From+To and all
//  other things (including the body) of a message intact
static BOOL WR_Redirect(FILE *fh, const struct Compose *comp)
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
      EmitHeader(fh, "Resent-From", BuildAddress(address, sizeof(address), comp->Identity->address, comp->Identity->realname), comp->codeset);
      EmitRcptHeader(fh, "Resent-To", comp->MailTo, comp->codeset);
      EmitRcptHeader(fh, "Resent-CC", comp->MailCC, comp->codeset);
      EmitRcptHeader(fh, "Resent-BCC", comp->MailBCC, comp->codeset);
      EmitHeader(fh, "Resent-Date", GetDateTime(), comp->codeset);
      NewMessageID(msgID, sizeof(msgID), comp->Identity->smtpServer);
      EmitHeader(fh, "Resent-Message-ID", msgID, comp->codeset);
      EmitHeader(fh, "Resent-User-Agent", yamuseragent, comp->codeset);

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
  "  AmigaOS...........: http://yam.ch/\n"
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
static BOOL WR_ComposePGP(FILE *fh, const struct Compose *comp, const char *boundary)
{
  enum Security sec = comp->Security;
  BOOL success = FALSE;
  struct WritePart pgppart;
  char *ids = dstralloc(SIZE_DEFAULT);
  char pgpfile[SIZE_PATHFILE];
  struct TempFile *tf;

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

    if(comp->Identity->pgpSelfEncrypt == TRUE &&
       comp->Identity->pgpKeyID[0] != '\0')
    {
      if(G->PGPVersion == 5)
        dstrcat(&ids, "-r ");

      dstrcat(&ids, comp->Identity->pgpKeyID);
    }
  }

  if((tf = OpenTempFile("w")) != NULL)
  {
    struct WritePart *firstpart = comp->FirstPart;

    if(EncodePart(tf->FP, firstpart) == FALSE)
      goto out;

    fclose(tf->FP);
    tf->FP = NULL;

    snprintf(pgpfile, sizeof(pgpfile), "%s.asc", tf->Filename);

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
          goto out;

        fprintf(fh, "\n"
                    "--%s\n"
                    "Content-Type: application/pgp-signature\n"
                    "\n",
                    boundary);

        snprintf(options, sizeof(options), (G->PGPVersion == 5) ? "-ab %s +batchmode=1 +force" : "-sab %s +bat +f", tf->Filename);

        if(comp->Identity->pgpKeyID[0] != '\0')
        {
          strlcat(options, " -u ", sizeof(options));
          strlcat(options, comp->Identity->pgpKeyID, sizeof(options));
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

        snprintf(options, sizeof(options), (G->PGPVersion == 5) ? "-a %s %s +batchmode=1 +force" : "-ea %s %s +bat +f", tf->Filename, ids);
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

        snprintf(options, sizeof(options), (G->PGPVersion == 5) ? "-a %s %s +batchmode=1 +force -s" : "-sea %s %s +bat +f", tf->Filename, ids);

        if(comp->Identity->pgpKeyID[0] != '\0')
        {
          strlcat(options, " -u ", sizeof(options));
          strlcat(options, comp->Identity->pgpKeyID, sizeof(options));
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
        goto out;
    }
  }

  if(pgpfile[0] != '\0')
    DeleteFile(pgpfile);

  fprintf(fh, "\n"
              "--%s--\n"
              "\n",
              boundary);

out:
  CloseTempFile(tf);
  dstrfree(ids);
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
  char address[SIZE_LARGE];

  ENTER();

  if(comp->Mode == NMM_REDIRECT)
  {
    if(comp->DelSend == TRUE)
      EmitHeader(fh, "X-YAM-Options", "delsent,redirect", comp->codeset);
    else
      EmitHeader(fh, "X-YAM-Options", "redirect", comp->codeset);

    success = WR_Redirect(fh, comp);

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

  // encrypted multipart message requested?
  if(firstpart->Next != NULL && comp->Security > SEC_NONE && comp->Security <= SEC_BOTH)
  {
    struct Compose tcomp;
    FILE *tfh;

    if((tf = OpenTempFile(NULL)) != NULL && (tfh = fopen(tf->Filename, "w")) != NULL)
    {
      setvbuf(tfh, NULL, _IOFBF, SIZE_FILEBUF);

      memcpy(&tcomp, comp, sizeof(tcomp)); // clone struct Compose
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
        if((comp->FirstPart = (struct WritePart *)calloc(1, sizeof(struct WritePart))) != NULL)
        {
          // reuse encoding
          comp->FirstPart->EncType = tpart->EncType;
          // free old parts list
          FreePartsList(tpart, TRUE);
          // the only part is an email message
          comp->FirstPart->ContentType = "message/rfc822";
          // set filename to tempfile
          comp->FirstPart->Filename = tf->Filename;
          // only use sig in enclosed mail
          comp->Signature = NULL;
        }
        else
        {
          // no errormsg here - the window probably won't open anyway...
          DisplayBeep(NULL);
          // just restore old parts list and switch off security
          comp->FirstPart = tpart;
          comp->Security = SEC_NONE;
          // we'll most likely get more errors further down :(
        }
      }
      else
      {
        ER_NewError(tr(MSG_ER_PGPMultipart));
        comp->Security = SEC_NONE;
      }

      fclose(tfh);
    }
    else
    {
      ER_NewError(tr(MSG_ER_PGPMultipart));
      comp->Security = SEC_NONE;
    }
  }

  // now we add a "X-YAM-Options" header to the mail so
  // that we can identify later certain options we have to set
  // if the same mail is being edited by the user again
  snprintf(buf, sizeof(buf), "signature=%08x,identity=%08x,security=%s%s", comp->Signature != NULL ? comp->Signature->id : 0,
                                                                           comp->Identity->id,
                                                                           SecCodes[comp->SelSecurity],
                                                                           comp->DelSend == TRUE ? ",delsent" : "");
  EmitHeader(fh, "X-YAM-Options", buf, comp->codeset);

  // add the "X-YAM-MailAccount" header to track over which SMTP server
  // this mail was sent
  snprintf(buf, sizeof(buf), "%s@%s", comp->Identity->smtpServer->username, comp->Identity->smtpServer->hostname);
  EmitHeader(fh, "X-YAM-MailAccount", buf, comp->codeset);

  // put the From: header entry into the mail
  if(comp->FromOverride != NULL && strcmp(comp->FromOverride, comp->Identity->address) != 0)
    EmitRcptHeader(fh, "From", BuildAddress(address, sizeof(address), comp->FromOverride, comp->Identity->realname), comp->codeset);
  else
    EmitRcptHeader(fh, "From", BuildAddress(address, sizeof(address), comp->Identity->address, comp->Identity->realname), comp->codeset);

  EmitRcptHeader(fh, "Reply-To", comp->ReplyTo, comp->codeset);
  EmitRcptHeader(fh, "To", comp->MailTo, comp->codeset);
  EmitRcptHeader(fh, "CC", comp->MailCC, comp->codeset);
  EmitRcptHeader(fh, "BCC", comp->MailBCC, comp->codeset);
  EmitRcptHeader(fh, "Mail-Reply-To", comp->MailReplyTo, comp->codeset);
  EmitRcptHeader(fh, "Mail-Followup-To", comp->MailFollowupTo, comp->codeset);

  // output the current date
  EmitHeader(fh, "Date", GetDateTime(), comp->codeset);

  // output the Message-ID, In-Reply-To and References message headers
  NewMessageID(msgID, sizeof(msgID), comp->Identity->smtpServer);
  EmitHeader(fh, "Message-ID", msgID, comp->codeset);
  EmitHeader(fh, "In-Reply-To", comp->inReplyToMsgID, comp->codeset);
  EmitHeader(fh, "References", comp->references, comp->codeset);

  if(comp->RequestMDN == TRUE)
  {
    if(comp->ReplyTo != NULL)
      EmitRcptHeader(fh, "Disposition-Notification-To", comp->ReplyTo, comp->codeset);
    else
      EmitRcptHeader(fh, "Disposition-Notification-To", comp->Identity->address, comp->codeset);
  }

  if(comp->Importance != 0)
    EmitHeader(fh, "Importance", comp->Importance == 1 ? "High" : "Low", comp->codeset);

  fprintf(fh, "User-Agent: %s\n", yamuseragent);

  if(comp->UserInfo == TRUE)
    WR_WriteUserInfo(fh, comp);

  // if the PGP key ID is set we go and output the OpenPGP header
  // field which is defined at http://josefsson.org/openpgp-header/
  if(comp->Identity->pgpKeyID[0] != '\0')
  {
    char *p = strchr(comp->Identity->pgpKeyID, 'x');
    if(p == NULL)
      p = comp->Identity->pgpKeyID;
    else
      p++;

    fprintf(fh, "OpenPGP: id=%s", p);

    if(comp->Identity->pgpKeyURL[0] != '\0')
    {
      fputc(';', fh);
      HeaderFputs(fh, comp->Identity->pgpKeyURL, "url", 0, comp->codeset);
    }

    fputc('\n', fh);
  }

  if(comp->Identity->organization[0] != '\0')
    EmitHeader(fh, "Organization", comp->Identity->organization, comp->codeset);

  if(comp->Subject[0] != '\0')
    EmitHeader(fh, "Subject", comp->Subject, comp->codeset);

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
/// AppendRcpt()
//  Appends a recipient address to a string
static char *AppendRcpt(char *sbuf, const struct Person *pe,
                        const struct UserIdentityNode *uin, const BOOL excludeme)
{
  ENTER();

  if(pe != NULL)
  {
    D(DBF_MAIL, "add recipient for person named '%s', address '%s', %s address '%s'", SafeStr(pe->RealName), SafeStr(pe->Address), excludeme ? "excluding" : "including", uin != NULL ? uin->address : "NULL");

    // Make sure that the person has at least either name or address and
    // that these are non-empty strings. Otherwise we will add invalid
    // recipients like '@domain' without any real name and user name.
    if(IsStrEmpty(pe->Address) == FALSE || IsStrEmpty(pe->RealName) == FALSE)
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
        // address does not contain any @ nor domain, lets add it
        char addr[SIZE_ADDRESS];
        char *p = NULL;

        if(uin != NULL)
          p = strchr(uin->address, '@');

        snprintf(addr, sizeof(addr), "%s%s", pe->Address, p != NULL ? p : "");
        ins = BuildAddress(address, sizeof(address), addr, pe->RealName);
      }

      if(ins != NULL)
      {
        // exclude the given person if it is ourself
        if(excludeme == TRUE && uin != NULL && stricmp(pe->Address, uin->address) == 0)
          skip = TRUE;

        // if the string already contains this person then skip it
        if(skip == FALSE && sbuf != NULL && strcasestr(sbuf, ins) != NULL)
          skip = TRUE;

        if(skip == FALSE)
        {
          D(DBF_MAIL, "adding recipient '%s'", ins);

          // prepend a ", " sequence in case sbuf is not empty
          if(IsStrEmpty(sbuf) == FALSE)
            dstrcat(&sbuf, ", ");

          dstrcat(&sbuf, ins);
        }
        else
        {
          D(DBF_MAIL, "skipping recipient '%s'", ins);
        }
      }
    }
    else
    {
      W(DBF_MAIL, "neither name nor address to build a recipient");
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
  char *dst = dstralloc(SIZE_DEFAULT);

  ENTER();

  for(; *src; src++)
  {
    if(*src == '\\')
    {
      src++;
      switch(*src)
      {
        case '\\':
          dstrcat(&dst, "\\");
        break;

        case 'n':
          dstrcat(&dst, "\n");
        break;
      }
    }
    else if(*src == '%' && etd != NULL)
    {
      src++;
      switch(*src)
      {
        case 'n':
          dstrcat(&dst, etd->OS_Name);
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
          dstrcat(&dst, p);
        }
        break;

        case 's':
          dstrcat(&dst, etd->OM_Subject);
        break;

        case 'e':
          dstrcat(&dst, etd->OS_Address);
        break;

        case 'd':
        {
          char datstr[64];

          DateStamp2String(datstr, sizeof(datstr), &etd->OM_Date, DSS_DATE, TZC_NONE);
          dstrcat(&dst, datstr);
        }
        break;

        case 't':
        {
          char datstr[64];

          DateStamp2String(datstr, sizeof(datstr), &etd->OM_Date, DSS_TIME, TZC_NONE);
          dstrcat(&dst, datstr);
        }
        break;

        case 'z':
        {
          char tzone[SIZE_SMALL];
          int convertedGmtOffset = (etd->OM_gmtOffset/60)*100 + (etd->OM_gmtOffset%60);

          if(IsStrEmpty(etd->OM_tzAbbr) == FALSE)
            snprintf(tzone, sizeof(tzone), "%+05d (%s)", convertedGmtOffset, etd->OM_tzAbbr);
          else
            snprintf(tzone, sizeof(tzone), "%+05d", convertedGmtOffset);

          dstrcat(&dst, tzone);
        }
        break;

        case 'w':
        {
          char datstr[64];

          DateStamp2String(datstr, sizeof(datstr), &etd->OM_Date, DSS_WEEKDAY, TZC_NONE);
          dstrcat(&dst, datstr);
        }
        break;

        case 'c':
        {
          char datstr[64];

          DateStamp2RFCString(datstr, sizeof(datstr), &etd->OM_Date, etd->OM_gmtOffset, etd->OM_tzAbbr, FALSE);
          dstrcat(&dst, datstr);
        }
        break;

        case 'm':
          dstrcat(&dst, etd->OM_MessageID);
        break;

        case 'r':
          dstrcat(&dst, etd->R_Name);
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

          dstrcat(&dst, p);
        }
        break;

        case 'a':
          dstrcat(&dst, etd->R_Address);
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
          dstrcat(&dst, buf);
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
          dstrcat(&dst, buf);
        }
        break;

        case 'h':
        {
          if((p = FileToBuffer(etd->HeaderFile, NULL)) != NULL)
          {
            dstrcat(&dst, p);
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
       dstrcat(&dst, chr);
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
      dstrfree(sbuf);
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
  etd->OM_gmtOffset= (mail != NULL) ? mail->gmtOffset : G->gmtOffset;
  etd->OM_tzAbbr   = (mail != NULL) ? mail->tzAbbr : G->tzAbbr;
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

    if(mail->gmtOffset != 0)
    {
      struct DateStamp *date = &etd->OM_Date;

      date->ds_Minute += mail->gmtOffset;

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
static struct WriteMailData *CreateWriteWindow(const enum NewMailMode mailMode, const BOOL quietMode)
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

    DoMethod(G->App, MUIM_YAMApplication_DisposeWindow, newWriteWindow);
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
  struct Folder *folder = GetCurrentFolder();
  struct WriteMailData *wmData = NULL;

  ENTER();

  // First check if the basic configuration is okay, then open write window */
  if(folder != NULL && IsValidConfig(C) == TRUE &&
     (wmData = CreateWriteWindow(NMM_NEW, quiet)) != NULL)
  {
    FILE *out;
    struct UserIdentityNode *userIdentity;

    // get the default user identity as a fallback
    userIdentity = GetUserIdentity(&C->userIdentityList, 0, TRUE);

    // open a new file for writing the default mail text
    if((out = fopen(wmData->filename, "w")) != NULL)
    {
      char *toAddr = dstralloc(SIZE_ADDRESS);
      char *replyToAddr = dstralloc(SIZE_ADDRESS);

      setvbuf(out, NULL, _IOFBF, SIZE_FILEBUF);

      // set either the draft or the reference mail pointer accordingly
      if(mail != NULL && isDraftsFolder(mail->Folder))
        wmData->draftMail = mail;
      else
        wmData->refMail = mail;

      if(mail != NULL)
      {
        struct ExtendedMail *email;

        // check whether the old mail contains a ReplyTo: address
        // or not. And if so we prefer that one instead of using the
        // To: adresses
        if(mail->ReplyTo.Address[0] != '\0')
        {
          // add the Reply-To: address as the new To: address
          toAddr = AppendRcpt(toAddr, &mail->ReplyTo, wmData->identity, FALSE);

          // if the mail has multiple reply-to recipients
          // we have to get them and add them as well
          if(isMultiReplyToMail(mail) &&
             (email = MA_ExamineMail(mail->Folder, mail->MailFile, TRUE)) != NULL)
          {
            int i;

            // add all "ReplyTo:" recipients of the mail
            for(i=0; i < email->NumSReplyTo; i++)
              toAddr = AppendRcpt(toAddr, &email->SReplyTo[i], email->identity, FALSE);

            // save the user identity
            userIdentity = email->identity;

            MA_FreeEMailStruct(email);
          }
        }
        else
        {
          // add the From: address as the new To: address
          toAddr = AppendRcpt(toAddr, &mail->From, wmData->identity, FALSE);

          // if the mail has multiple From: recipients
          // we have to get them and add them as well
          if(isMultiSenderMail(mail) &&
            (email = MA_ExamineMail(mail->Folder, mail->MailFile, TRUE)) != NULL)
          {
            int i;

            // add all "From:" recipients of the mail
            for(i=0; i < email->NumSFrom; i++)
              toAddr = AppendRcpt(toAddr, &email->SFrom[i], email->identity, FALSE);

            // save the user identity
            userIdentity = email->identity;

            MA_FreeEMailStruct(email);
          }
        }
      }
      else if(folder->MLSupport == TRUE)
      {
        if(folder->MLAddress[0] != '\0')
          dstrcpy(&toAddr, folder->MLAddress);

        if(folder->MLIdentity != NULL)
          userIdentity = folder->MLIdentity;

        if(folder->MLReplyToAddress[0] != '\0')
          dstrcpy(&replyToAddr, folder->MLReplyToAddress);
      }

      if(folder->WriteIntro[0] != '\0')
        InsertIntroText(out, folder->WriteIntro, NULL);
      else
        InsertIntroText(out, C->NewIntro, NULL);

      if(folder->WriteGreetings[0] != '\0')
        InsertIntroText(out, folder->WriteGreetings, NULL);
      else
        InsertIntroText(out, C->Greetings, NULL);

      // make sure to set the correct user identity
      xset(wmData->window, MUIA_WriteWindow_Identity, userIdentity,
                           toAddr[0] != '\0' ? MUIA_WriteWindow_To : TAG_IGNORE, toAddr,
                           replyToAddr[0] != '\0' ? MUIA_WriteWindow_ReplyTo : TAG_IGNORE, replyToAddr);

      // add a signature to the mail depending on the selected signature for this list
      WriteSignature(out, folder->MLSupport ? folder->MLSignature : userIdentity->signature, TRUE);

      // close the output file handle
      fclose(out);

      // update the message text
      DoMethod(wmData->window, MUIM_WriteWindow_LoadText, NULL, FALSE);

      // make sure the window is opened
      if(quiet == FALSE)
        SafeOpenWindow(wmData->window);

      // start with an unmodified message
      set(wmData->window, MUIA_WriteWindow_Modified, FALSE);

      if(C->LaunchAlways == TRUE && quiet == FALSE)
        DoMethod(wmData->window, MUIM_WriteWindow_LaunchEditor);

      dstrfree(toAddr);
      dstrfree(replyToAddr);
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

  // check if the mail in question resists in the drafts
  // folder
  if(isDraftsFolder(folder))
  {
    // search through our WriteMailDataList
    struct WriteMailData *lwmData;

    IterateList(&G->writeMailDataList, struct WriteMailData *, lwmData)
    {
      if(lwmData->window != NULL && lwmData->refMail == mail)
      {
        DoMethod(lwmData->window, MUIM_Window_ToFront);

        RETURN(lwmData);
        return lwmData;
      }
    }
  }

  // check if necessary settings for writing are OK and open new window
  if(IsValidConfig(C) == TRUE &&
     (wmData = CreateWriteWindow(isDraftsFolder(folder) ? NMM_EDIT : NMM_EDITASNEW, quiet)) != NULL)
  {
    FILE *out;

    if((out = fopen(wmData->filename, "w")) != NULL)
    {
      struct ReadMailData *rmData;
      struct ExtendedMail *email;
      char *p;

      setvbuf(out, NULL, _IOFBF, SIZE_FILEBUF);

      // set either the draft or the reference mail pointer accordingly
      if(isDraftsFolder(mail->Folder))
        wmData->draftMail = mail;
      else
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
          size_t msglen = dstrlen(cmsg);

          // we check whether cmsg contains any text and if so we
          // write out the whole text to our temporary file.
          if(msglen == 0 || fwrite(cmsg, msglen, 1, out) == 1)
          {
            int i;
            char address[SIZE_LARGE];
            BOOL reuseReplyToAddress = TRUE;
            char *sbuf = NULL;

            // free our temp text now
            dstrfree(cmsg);

            // set the In-Reply-To / References message header references, if they exist
            if(email->inReplyToMsgID != NULL)
            {
              D(DBF_MAIL, "adding inReplyToMsgID '%s'", email->inReplyToMsgID);
              dstrcpy(&wmData->inReplyToMsgID, email->inReplyToMsgID);
            }

            if(email->references != NULL)
            {
              D(DBF_MAIL, "adding references '%s'", email->references);
              dstrcpy(&wmData->references, email->references);
            }

            // set the subject gadget
            set(wmData->window, MUIA_WriteWindow_Subject, mail->Subject);

            // in case this is a EDITASNEW action with a mailing list
            // folder we have to make sure to add the From: and ReplyTo:
            // address of the mailing list
            if(wmData->mode == NMM_EDITASNEW && folder->MLSupport == TRUE)
            {
              if(folder->MLIdentity != NULL)
                email->identity = folder->MLIdentity;

              if(folder->MLReplyToAddress[0] != '\0')
              {
                set(wmData->window, MUIA_WriteWindow_ReplyTo, folder->MLReplyToAddress);
                reuseReplyToAddress = FALSE;
              }
            }

            // we now set the user identity in the write window (either the one
            // ExamineMail found out for us or the one we have overwritten
            // due to the MLSupport
            set(wmData->window, MUIA_WriteWindow_Identity, email->identity);
            set(wmData->window, MUIA_WriteWindow_From, mail->From.Address);

            if(reuseReplyToAddress == TRUE)
            {
              // add all ReplyTo: recipients
              D(DBF_MAIL, "adding ReplyTo recipient '%s'", mail->ReplyTo.Address);
              dstrcpy(&sbuf, BuildAddress(address, sizeof(address), mail->ReplyTo.Address, mail->ReplyTo.RealName));
              for(i=0; i < email->NumSReplyTo; i++)
              {
                D(DBF_MAIL, "adding ReplyTo recipient '%s'", email->SReplyTo[i].Address);
                sbuf = AppendRcpt(sbuf, &email->SReplyTo[i], email->identity, FALSE);
              }
              set(wmData->window, MUIA_WriteWindow_ReplyTo, sbuf);
            }

            // add all "To:" recipients of the mail
            D(DBF_MAIL, "adding To recipient '%s'", mail->To.Address);
            dstrcpy(&sbuf, BuildAddress(address, sizeof(address), mail->To.Address, mail->To.RealName));
            for(i=0; i < email->NumSTo; i++)
            {
              D(DBF_MAIL, "adding To recipient '%s'", email->STo[i].Address);
              sbuf = AppendRcpt(sbuf, &email->STo[i], email->identity, FALSE);
            }
            set(wmData->window, MUIA_WriteWindow_To, sbuf);

            // add all "CC:" recipients of the mail
            dstrreset(sbuf);
            for(i=0; i < email->NumCC; i++)
            {
              D(DBF_MAIL, "adding CC recipient '%s'", email->CC[i].Address);
              sbuf = AppendRcpt(sbuf, &email->CC[i], email->identity, FALSE);
            }
            set(wmData->window, MUIA_WriteWindow_CC, sbuf);

            // add all "BCC:" recipients of the mail
            dstrreset(sbuf);
            for(i=0; i < email->NumBCC; i++)
            {
              D(DBF_MAIL, "adding BCC recipient '%s'", email->BCC[i].Address);
              sbuf = AppendRcpt(sbuf, &email->BCC[i], email->identity, FALSE);
            }
            set(wmData->window, MUIA_WriteWindow_BCC, sbuf);

            // free our temporary buffer
            dstrfree(sbuf);

            if(email->extraHeaders != NULL)
              set(wmData->window, MUIA_WriteWindow_ExtHeaders, email->extraHeaders);

            xset(wmData->window, MUIA_WriteWindow_DelSend,    email->DelSend,
                                 MUIA_WriteWindow_MDN,        isSendMDNMail(mail),
                                 MUIA_WriteWindow_AddInfo,    isSenderInfoMail(mail),
                                 MUIA_WriteWindow_Importance, getImportanceLevel(mail) == IMP_HIGH ? 0 : getImportanceLevel(mail)+1,
                                 MUIA_WriteWindow_Signature,  email->signature,
                                 MUIA_WriteWindow_Security,   email->Security);

            // setup the write window from an existing readmailData structure
            DoMethod(wmData->window, MUIM_WriteWindow_SetupFromOldMail, rmData);
          }
          else
          {
            E(DBF_MAIL, "Error while writing cmsg to out FH");

            // an error occurred while trying to write the text to out
            dstrfree(cmsg);
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
      DoMethod(wmData->window, MUIM_WriteWindow_LoadText, NULL, FALSE);

      // make sure the window is opened
      if(quiet == FALSE)
        SafeOpenWindow(wmData->window);

      // make sure that either the To:, Subject: or the
      // texteditor is the active object depending on which
      // string is empty
      p = (char *)xget(wmData->window, MUIA_WriteWindow_To);
      if(IsStrEmpty(p))
        set(wmData->window, MUIA_WriteWindow_ActiveObject, MUIV_WriteWindow_ActiveObject_To);
      else
      {
        p = (char *)xget(wmData->window, MUIA_WriteWindow_Subject);
        if(IsStrEmpty(p))
          set(wmData->window, MUIA_WriteWindow_ActiveObject, MUIV_WriteWindow_ActiveObject_Subject);
        else
          set(wmData->window, MUIA_WriteWindow_ActiveObject, MUIV_WriteWindow_ActiveObject_TextEditor);
      }

      // start with an unmodified message
      set(wmData->window, MUIA_WriteWindow_Modified, FALSE);

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
  if(IsValidConfig(C) == TRUE &&
     (wmData = CreateWriteWindow(NMM_FORWARD, quiet)) != NULL)
  {
    FILE *out;

    if((out = fopen(wmData->filename, "w")) != NULL)
    {
      struct SignatureNode *signature = NULL;
      int j = 0;
      enum ForwardMode fwdMode = C->ForwardMode;
      char *rsub = dstralloc(SIZE_SUBJECT);
      struct MailNode *mnode;
      struct UserIdentityNode *firstIdentity = NULL; // first identified identity over all mails

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
      SortMailList(mlist, CompareMailsByDate);

      // insert the intro text
      InsertIntroText(out, C->NewIntro, NULL);

      // iterate through all the mail in the
      // mail list and build up the forward text
      ForEachMailNode(mlist, mnode)
      {
        struct ExtendedMail *email;
        struct ExpandTextData etd;
        struct Mail *mail = mnode->mail;

        if(signature == NULL && mail->Folder != NULL &&
           mail->Folder->MLSupport == TRUE)
        {
          signature = mail->Folder->MLSignature;
        }

        if((email = MA_ExamineMail(mail->Folder, mail->MailFile, TRUE)) == NULL)
        {
          char mailfile[SIZE_PATHFILE];

          GetMailFile(mailfile, sizeof(mailfile), NULL, mail);
          ER_NewError(tr(MSG_ER_CantOpenFile), mailfile);
          fclose(out);
          dstrfree(rsub);

          CleanupWriteMailData(wmData);

          RETURN(NULL);
          return NULL;
        }

        // save the identity ptr if this is our first iteration
        if(firstIdentity == NULL)
          firstIdentity = email->identity;

        SetupExpandTextData(&etd, &email->Mail);
        etd.OM_MessageID = email->messageID;
        etd.R_Name = mail->To.RealName[0] != '\0' ? mail->To.RealName : mail->To.Address;
        etd.R_Address = mail->To.Address;

        // we create a generic subject line for the forward
        // action so that a forwarded mail will have a "Fwd: XXX" kinda
        // subject line instead of the original.
        if(mail->Subject != '\0')
        {
          char buffer[SIZE_LARGE];

          snprintf(buffer, sizeof(buffer), "Fwd: %s", mail->Subject);
          if(strstr(rsub, buffer) == NULL)
          {
            if(rsub[0] != '\0')
              dstrcat(&rsub, "; ");

            dstrcat(&rsub, buffer);
          }
        }

        // we add the footer at the front if the user
        // wants this
        if(j == 0)
        {
          if(firstIdentity->quotePosition == QPOS_ABOVE)
            InsertIntroText(out, C->Greetings, NULL);

          if(firstIdentity->sigForwarding == TRUE &&
             (firstIdentity->quotePosition == QPOS_ABOVE && firstIdentity->signaturePosition == SPOS_ABOVE))
          {
            WriteSignature(out, signature == NULL ? firstIdentity->signature : signature, FALSE);
            fputs("\n", out);
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
            // add the mail to our attachment list
            DoMethod(wmData->window, MUIM_WriteWindow_AddMailAttachment, mail);
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
                dstrfree(cmsg);

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

        j++;
      }

      // make sure the correct identity is set
      set(wmData->window, MUIA_WriteWindow_Identity, firstIdentity);

      // add some footer with greatings.
      if(firstIdentity->quotePosition == QPOS_BELOW)
        InsertIntroText(out, C->Greetings, NULL);

      // add a signature to the mail depending on the selected signature for this list
      if(firstIdentity->sigForwarding == TRUE &&
         (firstIdentity->quotePosition == QPOS_BELOW || firstIdentity->signaturePosition == SPOS_BELOW))
      {
        WriteSignature(out, signature == NULL ? firstIdentity->signature : signature, TRUE);
      }

      // close the output file handle
      fclose(out);

      // set the composed subject text
      set(wmData->window, MUIA_WriteWindow_Subject, rsub);
      dstrfree(rsub);

      // update the message text
      DoMethod(wmData->window, MUIM_WriteWindow_LoadText, NULL, FALSE);

      // make sure the window is opened
      if(quiet == FALSE)
        SafeOpenWindow(wmData->window);

      // set the active object of the window
      set(wmData->window, MUIA_WriteWindow_ActiveObject, MUIV_WriteWindow_ActiveObject_To);

      // start with an unmodified message
      set(wmData->window, MUIA_WriteWindow_Modified, FALSE);

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
/// FindMLIdentity
// check the given folder for mailing list support and return an appropriate user identity
static BOOL FindMLIdentity(struct Folder *folder, struct ExtendedMail *email, char **mlistTo, char **mlistReplyTo)
{
  BOOL result = FALSE;

  ENTER();

  if(folder->MLSupport == TRUE && folder->MLPattern[0] != '\0')
  {
    int k;

    // first, search in all To: addresses for the
    // mailing list address
    for(k=0; k < email->NumSTo+1; k++)
    {
      struct Person *person;

      if(k == 0)
        person = &email->Mail.To;
      else
        person = &email->STo[k-1];

      if(MatchNoCase(person->Address, folder->MLPattern) == TRUE ||
         MatchNoCase(person->RealName, folder->MLPattern) == TRUE)
      {
        result = TRUE;
        break;
      }
    }

    // second, search in all CC: addresses for the
    // mailing list address
    if(result == FALSE)
    {
      for(k=0; k < email->NumCC; k++)
      {
        struct Person *person = &email->CC[k];

        if(MatchNoCase(person->Address, folder->MLPattern) == TRUE ||
           MatchNoCase(person->RealName, folder->MLPattern) == TRUE)
        {
          result = TRUE;
          break;
        }
      }
    }

    if(result == TRUE)
    {
      *mlistTo = folder->MLAddress[0] != '\0' ? folder->MLAddress : NULL;

      if(folder->MLIdentity != NULL)
        email->identity = folder->MLIdentity;

      if(folder->MLReplyToAddress[0] != '\0')
        *mlistReplyTo = folder->MLReplyToAddress;
    }
  }

  RETURN(result);
  return result;
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
  if(IsValidConfig(C) == TRUE &&
     (wmData = CreateWriteWindow(NMM_REPLY, quiet)) != NULL)
  {
    FILE *out;

    if((out = fopen(wmData->filename, "w")) != NULL)
    {
      int j;
      int repmode = 1;
      struct SignatureNode *signature = NULL;
      BOOL altpat = FALSE;
      char *domain = NULL;
      struct UserIdentityNode *firstIdentity = NULL; // first identified identity over all mails
      char *rto = dstralloc(SIZE_ADDRESS);
      char *rcc = dstralloc(SIZE_ADDRESS);
      char *rrepto = dstralloc(SIZE_ADDRESS);
      char *rsub = dstralloc(SIZE_SUBJECT);
      char buffer[SIZE_LARGE];
      struct ExpandTextData etd;
      BOOL mlIntro = FALSE; // an mailing list intro text was added
      struct MailNode *mnode;

      setvbuf(out, NULL, _IOFBF, SIZE_FILEBUF);

      // make sure the write window know of the
      // operation and knows which mails to process
      wmData->refMailList = CloneMailList(mlist);

      // make sure we sort the mlist according to
      // the mail date
      SortMailList(mlist, CompareMailsByDate);

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
        char *mlistTo = NULL; // user-defined To: for mailing lists
        char *mlistReplyTo = NULL;  // user-defined Reply-To: for mailing lists

        if((email = MA_ExamineMail(folder, mail->MailFile, TRUE)) == NULL)
        {
          char mailfile[SIZE_PATHFILE];

          GetMailFile(mailfile, sizeof(mailfile), NULL, mail);
          ER_NewError(tr(MSG_ER_CantOpenFile), mailfile);
          fclose(out);
          CleanupWriteMailData(wmData);
          dstrfree(rto);
          dstrfree(rcc);
          dstrfree(rsub);

          RETURN(NULL);
          return NULL;
        }

        SHOWSTRING(DBF_MAIL, mail->ReplyTo.Address);
        SHOWSTRING(DBF_MAIL, mail->From.Address);
        SHOWVALUE(DBF_MAIL, email->NumMailReplyTo);
        SHOWVALUE(DBF_MAIL, email->NumFollowUpTo);

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
              dstrcat(&rsub, "; ");

            dstrcat(&rsub, buffer);
          }
        }

        // in case we are replying to a single message we also have to
        // save the messageID of the email we are replying to
        if(wmData->inReplyToMsgID != NULL)
          dstrcat(&wmData->inReplyToMsgID, " ");

        if(email->messageID != NULL)
        {
          D(DBF_MAIL, "adding inReplyToMsgID '%s'", email->inReplyToMsgID);
          dstrcat(&wmData->inReplyToMsgID, email->messageID);
        }

        // in addition, we check for "References:" message header stuff
        if(wmData->references != NULL)
          dstrcat(&wmData->references, " ");

        if(email->references != NULL)
        {
          D(DBF_MAIL, "adding references '%s'", email->references);
          dstrcat(&wmData->references, email->references);
        }
        else
        {
          // check if this email contains inReplyToMsgID data and if so we
          // create a new references header entry
          if(email->inReplyToMsgID != NULL)
          {
            D(DBF_MAIL, "adding references '%s'", email->inReplyToMsgID);
            dstrcat(&wmData->references, email->inReplyToMsgID);
          }
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
              struct Folder *curFolder = fnode->folder;

              if(FindMLIdentity(curFolder, email, &mlistTo, &mlistReplyTo) == TRUE)
              {
                foundMLFolder = TRUE;
                break;
              }
            }

            UnlockFolderList(G->folders);
          }
          else if(isSentMailFolder(folder))
          {
            foundSentFolder = TRUE;
          }
          else
          {
            foundMLFolder = FindMLIdentity(folder, email, &mlistTo, &mlistReplyTo);
          }
        }

        SHOWVALUE(DBF_MAIL, foundSentFolder);
        SHOWVALUE(DBF_MAIL, foundMLFolder);
        SHOWVALUE(DBF_MAIL, hasPrivateFlag(flags));
        SHOWVALUE(DBF_MAIL, hasMListFlag(flags));

        // If this mail is a standard multi-recipient mail and the user hasn't pressed SHIFT (private)
        // or ALT (send to mailing list) we going to ask him to which recipient he want to send the mail to.
        if((isMultiRCPTMail(mail) || (email->NumMailReplyTo > 0 && email->NumFollowUpTo > 0 && foundMLFolder == FALSE)) &&
            hasPrivateFlag(flags) == FALSE && hasMListFlag(flags) == FALSE)
        {
          const char *opt;

          // if we are here because of the Mail-Reply-To and Mail-Followup-To headers
          // we only present two options instead of three
          if(email->NumMailReplyTo > 0 && email->NumFollowUpTo > 0)
            opt = tr(MSG_MA_REPLYREQ_OPTIONS2);
          else
            opt = tr(MSG_MA_ReplyReqOpt);

          // ask the user and in case he want to abort, quit this
          // function immediately.
          if((repmode = MUI_Request(_app(wmData->window), G->MA->GUI.WI, MUIF_NONE, NULL, opt, tr(MSG_MA_ReplyReq))) == 0)
          {
            MA_FreeEMailStruct(email);
            fclose(out);
            CleanupWriteMailData(wmData);
            dstrfree(rto);
            dstrfree(rcc);
            dstrfree(rsub);

            RETURN(NULL);
            return NULL;
          }
        }

        // now we should know how the user wants to
        // reply to the mail. The possible reply modes are:
        //
        // repmode == 1 : To Sender (From:/ReplyTo: -> To:)
        // repmode == 2 : To Sender and all recipients (From:/ReplyTo: -> To:, To:+CC: -> CC:)
        // repmode == 3 : To Recipients (To:+CC: -> To:)
        SHOWVALUE(DBF_MAIL, repmode);
        switch(repmode)
        {
          // user wants to reply to sender of the
          // mail only or he didn't decide anything yet (default)
          case 1:
          {
            BOOL addDefault = FALSE;

            // the user wants to reply to the Sender (From:/ReplyTo:), however we
            // need to check whether he want to get asked or directly reply to
            // the wanted address.
            if(foundSentFolder == TRUE)
            {
              // the mail to which the user wants to reply is stored in a
              // sent mail folder. As such all mail should originate from ourself
              // and as such when he presses "reply" on it we send it to
              // the To: address recipient instead.
              D(DBF_MAIL, "adding To recipient '%s'", mail->To.Address);
              rto = AppendRcpt(rto, &mail->To, email->identity, FALSE);
              for(k=0; k < email->NumSTo; k++)
              {
                D(DBF_MAIL, "adding To recipient '%s'", email->STo[k].Address);
                rto = AppendRcpt(rto, &email->STo[k], email->identity, FALSE);
              }
            }
            else if(hasPrivateFlag(flags) == FALSE &&
                    foundMLFolder == TRUE)
            {
              char *p;

              // add all To: recipients of the mailing list config
              // to the actual rto list
              if(mlistTo != NULL && (p = strdup(mlistTo)) != NULL)
              {
                char *ptr = p;

                // we found a matching folder for the mail we are going to
                // reply to, so we go and add the 'mailTo' to our To: addresses
                while(IsStrEmpty(ptr) == FALSE)
                {
                  char *next;

                  if((next = MyStrChr(ptr, ',')) != NULL)
                    *next++ = '\0';

                  ExtractAddress(ptr, &pe);
                  D(DBF_MAIL, "adding To recipient '%s'", pe.Address);
                  rto = AppendRcpt(rto, &pe, email->identity, FALSE);

                  ptr = next;
                }

                free(p);
              }
              else
              {
                // the user hasn't specified a To: address so lets check which one to take
                if(email->NumFollowUpTo > 0)
                {
                  for(k=0; k < email->NumFollowUpTo; k++)
                  {
                    D(DBF_MAIL, "adding To recipient '%s'", email->FollowUpTo[k].Address);
                    rto = AppendRcpt(rto, &email->FollowUpTo[k], email->identity, FALSE);
                  }
                }
                else if(IsStrEmpty(mail->ReplyTo.Address) == FALSE)
                {
                  D(DBF_MAIL, "adding To recipient '%s'", mail->ReplyTo.Address);
                  rto = AppendRcpt(rto, &mail->ReplyTo, email->identity, FALSE);
                  for(k=0; k < email->NumSReplyTo; k++)
                  {
                    D(DBF_MAIL, "adding To recipient '%s'", email->SReplyTo[k].Address);
                    rto = AppendRcpt(rto, &email->SReplyTo[k], email->identity, FALSE);
                  }
                }
              }

              // add all Reply-To: recipients of the mailing list config
              // to the actual rrepto list
              if(mlistReplyTo != NULL && (p = strdup(mlistReplyTo)) != NULL)
              {
                char *ptr = p;

                // we found a matching folder for the mail we are going to
                // reply to, so we go and add the 'mailReplyTo' to our Reply-To: addresses
                while(IsStrEmpty(ptr) == FALSE)
                {
                  char *next;

                  if((next = MyStrChr(ptr, ',')) != NULL)
                    *next++ = '\0';

                  ExtractAddress(ptr, &pe);
                  D(DBF_MAIL, "adding ReplyTo recipient '%s'", pe.Address);
                  rrepto = AppendRcpt(rrepto, &pe, email->identity, FALSE);

                  ptr = next;
                }

                free(p);
              }
            }
            else if(C->CompareAddress == TRUE &&
                    (hasMListFlag(flags) == FALSE || hasPrivateFlag(flags) == TRUE) &&
                    (IsStrEmpty(mail->ReplyTo.Address) == FALSE || email->NumMailReplyTo > 0))
            {
              BOOL askUser = FALSE;

              // Mail-Reply-To: takes preference over the plain Reply-To: header.
              // Thus we check for that one first
              if(email->NumMailReplyTo > 0)
              {
                // force to use the Mail-Reply-To: header without asking anyone
                askUser = FALSE;
              }
              else if(hasPrivateFlag(flags) == TRUE)
              {
                // the user don't want to be asked if he presses the SHIT key while replying
                // so skip any requester
                askUser = FALSE;
              }
              else if(email->NumSFrom == email->NumSReplyTo)
              {
                // now we have to check whether the ReplyTo: and From: of the original
                // message are the very same or not.
                for(k=0; k < email->NumSFrom+1; k++)
                {
                  int diff;

                  if(k == 0)
                    diff = stricmp(mail->From.Address, mail->ReplyTo.Address);
                  else
                    diff = stricmp(email->SFrom[k-1].Address, email->SReplyTo[k-1].Address);

                  if(diff != 0)
                  {
                    askUser = TRUE;
                    break;
                  }
                }
              }
              else
                askUser = TRUE;

              // if askUser == TRUE, we go and
              // ask the user which address he wants to reply to.
              if(askUser == TRUE)
              {
                snprintf(buffer, sizeof(buffer), tr(MSG_MA_CompareReq), mail->From.Address, mail->ReplyTo.Address);

                switch(MUI_Request(_app(wmData->window), G->MA->GUI.WI, MUIF_NONE, NULL, tr(MSG_MA_Compare3ReqOpt), buffer))
                {
                  // Both (From:+ReplyTo:/Mail-Reply-To:) address
                  case 3:
                  {
                    // add all From: addresses to the CC: list
                    D(DBF_MAIL, "adding CC recipient '%s'", mail->From.Address);
                    rcc = AppendRcpt(rcc, &mail->From, email->identity, FALSE);
                    for(k=0; k < email->NumSFrom; k++)
                    {
                      D(DBF_MAIL, "adding CC recipient '%s'", email->SFrom[k].Address);
                      rcc = AppendRcpt(rcc, &email->SFrom[k], email->identity, FALSE);
                    }
                  }
                  // continue

                  // Reply-To:/Mail-Reply-To: addresses
                  case 2:
                  {
                    addDefault = TRUE;
                  }
                  break;

                  // only From: addresses
                  case 1:
                  {
                    D(DBF_MAIL, "adding To recipient '%s'", mail->From.Address);
                    rto = AppendRcpt(rto, &mail->From, email->identity, FALSE);
                    for(k=0; k < email->NumSFrom; k++)
                    {
                      D(DBF_MAIL, "adding To recipient '%s'", email->SFrom[k].Address);
                      rto = AppendRcpt(rto, &email->SFrom[k], email->identity, FALSE);
                    }
                  }
                  break;

                  // cancel operation
                  case 0:
                  {
                    MA_FreeEMailStruct(email);
                    fclose(out);
                    CleanupWriteMailData(wmData);
                    dstrfree(rto);
                    dstrfree(rcc);
                    dstrfree(rrepto);
                    dstrfree(rsub);

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
              D(DBF_MAIL, "add default To recipients");
              // we first check if there is a Mail-Reply-To: mail header and if so we
              // use that instead of any Reply-To: or From: mail header. Otherwise we
              // use the Reply-To: recipient list and if that is also not present we
              // use the From: recipient list only.
              if(email->NumMailReplyTo > 0 && hasMListFlag(flags) == FALSE)
              {
                for(k=0; k < email->NumMailReplyTo; k++)
                {
                  D(DBF_MAIL, "adding To recipient '%s'", email->MailReplyTo[k].Address);
                  rto = AppendRcpt(rto, &email->MailReplyTo[k], email->identity, FALSE);
                }
              }
              else if(email->NumFollowUpTo > 0 && hasMListFlag(flags) == TRUE)
              {
                for(k=0; k < email->NumFollowUpTo; k++)
                {
                  D(DBF_MAIL, "adding To recipient '%s'", email->FollowUpTo[k].Address);
                  rto = AppendRcpt(rto, &email->FollowUpTo[k], email->identity, FALSE);
                }
              }
              else if(IsStrEmpty(mail->ReplyTo.Address) == FALSE && hasPrivateFlag(flags) == FALSE)
              {
                D(DBF_MAIL, "adding To recipient '%s'", mail->ReplyTo.Address);
                rto = AppendRcpt(rto, &mail->ReplyTo, email->identity, FALSE);
                for(k=0; k < email->NumSReplyTo; k++)
                {
                  D(DBF_MAIL, "adding To recipient '%s'", email->SReplyTo[k].Address);
                  rto = AppendRcpt(rto, &email->SReplyTo[k], email->identity, FALSE);
                }
              }
              else
              {
                D(DBF_MAIL, "adding To recipient '%s'", mail->From.Address);
                rto = AppendRcpt(rto, &mail->From, email->identity, FALSE);
                for(k=0; k < email->NumSFrom; k++)
                {
                  D(DBF_MAIL, "adding To recipient '%s'", email->SFrom[k].Address);
                  rto = AppendRcpt(rto, &email->SFrom[k], email->identity, FALSE);
                }
              }
            }
          }
          break;

          // user wants to reply to all senders and recipients
          case 2:
          {
            // user wants to 'reply-to-all'
            // here Mail-Followup-To: has preference over Reply-To: or the
            // content of the From: recipient list
            if(email->NumFollowUpTo > 0)
            {
              for(k=0; k < email->NumFollowUpTo; k++)
              {
                D(DBF_MAIL, "adding To recipient '%s'", email->FollowUpTo[k].Address);
                rto = AppendRcpt(rto, &email->FollowUpTo[k], email->identity, FALSE);
              }
            }
            else
            {
              if(email->NumMailReplyTo > 0)
              {
                for(k=0; k < email->NumMailReplyTo; k++)
                {
                  D(DBF_MAIL, "adding To recipient '%s'", email->MailReplyTo[k].Address);
                  rto = AppendRcpt(rto, &email->MailReplyTo[k], email->identity, FALSE);
                }
              }
              else if(mail->ReplyTo.Address[0] != '\0')
              {
                // add Reply-To: addresses to To:
                D(DBF_MAIL, "adding To recipient '%s'", mail->ReplyTo.Address);
                rto = AppendRcpt(rto, &mail->ReplyTo, email->identity, FALSE);
                for(k=0; k < email->NumSReplyTo; k++)
                {
                  D(DBF_MAIL, "adding To recipient '%s'", email->SReplyTo[k].Address);
                  rto = AppendRcpt(rto, &email->SReplyTo[k], email->identity, FALSE);
                }
              }
              else
              {
                // add From: addresses to To:
                D(DBF_MAIL, "adding To recipient '%s'", mail->From.Address);
                rto = AppendRcpt(rto, &mail->From, email->identity, FALSE);
                for(k=0; k < email->NumSFrom; k++)
                {
                  D(DBF_MAIL, "adding To recipient '%s'", email->SFrom[k].Address);
                  rto = AppendRcpt(rto, &email->SFrom[k], email->identity, FALSE);
                }
              }

              // add To: addresses to CC:
              D(DBF_MAIL, "adding CC recipient '%s'", mail->To.Address);
              rcc = AppendRcpt(rcc, &mail->To, email->identity, TRUE);
              for(k=0; k < email->NumSTo; k++)
              {
                D(DBF_MAIL, "adding CC recipient '%s'", email->STo[k].Address);
                rcc = AppendRcpt(rcc, &email->STo[k], email->identity, TRUE);
              }
              for(k=0; k < email->NumCC; k++)
              {
                D(DBF_MAIL, "adding CC recipient '%s'", email->CC[k].Address);
                rcc = AppendRcpt(rcc, &email->CC[k], email->identity, TRUE);
              }
            }
          }
          break;

          // user wants to reply to all
          // recipients of the mail
          case 3:
          {
            // now add all original To: addresses to To:
            D(DBF_MAIL, "adding To recipient '%s'", mail->To.Address);
            rto = AppendRcpt(rto, &mail->To, email->identity, TRUE);
            for(k=0; k < email->NumSTo; k++)
            {
              D(DBF_MAIL, "adding To recipient '%s'", email->STo[k].Address);
              rto = AppendRcpt(rto, &email->STo[k], email->identity, TRUE);
            }
            // add the CC: addresses as well
            for(k=0; k < email->NumCC; k++)
            {
              D(DBF_MAIL, "adding CC recipient '%s'", email->CC[k].Address);
              rcc = AppendRcpt(rcc, &email->CC[k], email->identity, TRUE);
            }
          }
          break;
        }

        // extract the first address/name from our generated
        // To: address string
        ExtractAddress(rto, &pe);
        etd.R_Name = pe.RealName;
        etd.R_Address = pe.Address;

        // extract the domain name from the To address or respective
        // the default To: mail address
        if((domain = strchr(pe.Address, '@')) == NULL)
          domain = strchr(wmData->identity->address, '@');

        if(C->AltReplyPattern[0] != '\0' && domain != NULL && MatchNoCase(domain, C->AltReplyPattern))
          altpat = TRUE;
        else
          altpat = FALSE;

        // save the identity ptr if this is our first iteration
        if(firstIdentity == NULL)
          firstIdentity = email->identity;

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
          // if the answer should be put in front of quoted text
          // we need to insert the Bye text right away
          if(firstIdentity->quotePosition == QPOS_ABOVE)
            InsertIntroText(out, mlIntro ? C->MLReplyBye : (altpat ? C->AltReplyBye: C->ReplyBye), &etd);

          if(firstIdentity->sigReply == TRUE &&
             (firstIdentity->quotePosition == QPOS_ABOVE && firstIdentity->signaturePosition == SPOS_ABOVE))
          {
            WriteSignature(out, signature == NULL ? firstIdentity->signature : signature, FALSE);
            fputs("\n", out);
          }
        }

        // if the user wants to quote the mail text of the original mail,
        // we process it right now.
        if(firstIdentity->quoteMails == TRUE && !hasNoQuoteFlag(flags))
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
            if(IsStrEmpty(replytxt) == FALSE)
            {
              // make sure we quote the text in question.
              QuoteText(out, replytxt, strlen(replytxt), C->EdWrapMode != EWM_OFF ? C->EdWrapCol-2 : 1024);
            }
            else if((cmsg = RE_ReadInMessage(rmData, RIM_QUOTE)) != NULL)
            {
              // make sure we quote the text in question.
              QuoteText(out, cmsg, dstrlen(cmsg), C->EdWrapMode != EWM_OFF ? C->EdWrapCol-2 : 1024);

              dstrfree(cmsg);
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
          dstrcat(&wmData->references, " ");

        dstrcat(&wmData->references, wmData->inReplyToMsgID);
      }

      // make sure the correct identity has been set (if multiple mails
      // had been selected only the first one will be considered, thought)
      // and also set other important GUI elements with information we
      // collected here.
      xset(wmData->window, MUIA_WriteWindow_Identity, firstIdentity,
                           rto[0] != '\0' ? MUIA_WriteWindow_To      : TAG_IGNORE, rto,
                           rsub[0] != '\0'? MUIA_WriteWindow_Subject : TAG_IGNORE, rsub,
                           rcc[0] != '\0' ? (rto[0] != '\0' ? MUIA_WriteWindow_CC : MUIA_WriteWindow_To) : TAG_IGNORE, rcc,
                           rrepto[0] != '\0' ? MUIA_WriteWindow_ReplyTo : TAG_IGNORE, rrepto);

      // now that the mail is finished, we go and output some footer message to
      // the reply text. But we only do this if the user hasn't selected to have
      // his text ABOVE the quotes
      if(firstIdentity->quotePosition == QPOS_BELOW)
        InsertIntroText(out, mlIntro ? C->MLReplyBye : (altpat ? C->AltReplyBye: C->ReplyBye), &etd);

      // add a signature to the mail depending on the selected signature for this list
      if(firstIdentity->sigReply == TRUE &&
         (firstIdentity->quotePosition == QPOS_BELOW || firstIdentity->signaturePosition == SPOS_BELOW))
      {
        WriteSignature(out, signature == NULL ? firstIdentity->signature : signature, TRUE);
      }

      // close the filehandle
      fclose(out);

      // update the message text
      DoMethod(wmData->window, MUIM_WriteWindow_LoadText, NULL, FALSE);

      // make sure the window is opened
      if(quiet == FALSE)
        SafeOpenWindow(wmData->window);

      // set the active object of the window
      set(wmData->window, MUIA_WriteWindow_ActiveObject, MUIV_WriteWindow_ActiveObject_TextEditor);

      // start with an unmodified message
      set(wmData->window, MUIA_WriteWindow_Modified, FALSE);

      if(C->LaunchAlways == TRUE && quiet == FALSE)
        DoMethod(wmData->window, MUIM_WriteWindow_LaunchEditor);

      // free our temporary buffers
      dstrfree(rto);
      dstrfree(rcc);
      dstrfree(rrepto);
      dstrfree(rsub);
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
/// NewRedirectMailWindow()
//  Redirect message(s) to another recipient
struct WriteMailData *NewRedirectMailWindow(struct MailList *mlist, const int flags)
{
  BOOL quiet = hasQuietFlag(flags);
  struct WriteMailData *wmData = NULL;

  ENTER();

  // check if necessary settings fror writing are OK and open new window
  if(IsValidConfig(C) == TRUE &&
     (wmData = CreateWriteWindow(NMM_REDIRECT, quiet)) != NULL)
  {
    // make sure the write window know of the
    // operation and knows which mails to process
    wmData->refMailList = CloneMailList(mlist);

    // make sure the window is opened
    if(quiet == FALSE)
      SafeOpenWindow(wmData->window);

    // set the active object of the window
    set(wmData->window, MUIA_WriteWindow_ActiveObject, MUIV_WriteWindow_ActiveObject_To);

    // start with an unmodified message
    set(wmData->window, MUIA_WriteWindow_Modified, FALSE);
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

  if((wmData = AllocSysObjectTags(ASOT_NODE,
    ASONODE_Size, sizeof(*wmData),
    ASONODE_Min, TRUE,
    TAG_DONE)) != NULL)
  {
    memset(wmData, 0, sizeof(*wmData));
    wmData->mode = NMM_NEW;
    DateStamp(&wmData->lastFileChangeTime);

    // per default the writemaildata ptr to the user identity
    // links to the first active user identity in the list
    wmData->identity = GetUserIdentity(&C->userIdentityList, 0, TRUE);
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
    dstrfree(wmData->inReplyToMsgID);
    wmData->inReplyToMsgID = NULL;
  }

  if(wmData->references != NULL)
  {
    dstrfree(wmData->references);
    wmData->references = NULL;
  }

  // clean up the write window now
  if(wmData->window != NULL)
  {
    // make sure the window is really closed
    D(DBF_GUI, "make sure the write window is closed");
    nnset(wmData->window, MUIA_Window_Open, FALSE);

    D(DBF_GUI, "cleaning up write window");
    DoMethod(_app(wmData->window), MUIM_YAMApplication_DisposeWindow, wmData->window);

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
  struct WriteMailData *wmData;

  ENTER();

  IterateList(&G->writeMailDataList, struct WriteMailData *, wmData)
  {
    if(wmData->refMail == search)
    {
      wmData->refMail = (struct Mail *)newRef;
      result = TRUE;
    }

    if(wmData->refMailList != NULL)
    {
      struct MailNode *mnode;

      LockMailListShared(wmData->refMailList);

      if((mnode = FindMailByAddress(wmData->refMailList, search)) != NULL)
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
/// CreateHashTable
//  Creates an index table for a database file
static BOOL CreateHashTable(char *source, char *hashfile, char *sep)
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
      char *buf = NULL;
      size_t buflen = 0;
      const size_t seplen = strlen(sep);

      setvbuf(out, NULL, _IOFBF, SIZE_FILEBUF);

      // the first offset is always zero
      WriteUInt32(out, 0);
      while(getline(&buf, &buflen, in) > 0)
      {
        // if we found a separator write out the current offset
        if(strncmp(buf, sep, seplen) == 0)
          WriteUInt32(out, ftell(in));
      }

      fclose(out);

      free(buf);

      result = TRUE;
    }

    fclose(in);
  }

  RETURN(result);
  return result;
}

///
/// AddTagline
//  Randomly selects a tagline and writes it to the message file
static void AddTagline(FILE *fh_mail)
{
  ENTER();

  if(C->TagsFile[0] != '\0')
  {
    char hashfile[SIZE_PATHFILE];
    LONG tagsTime;
    LONG hashTime;
    BOOL createHashFile;
    FILE *fh_tag;

    snprintf(hashfile, sizeof(hashfile), "%s.hsh", C->TagsFile);

    if(FileExists(hashfile) == FALSE)
    {
      // create the .hsh file if it doesn't exist
      createHashFile = TRUE;
    }
    else if(ObtainFileInfo(C->TagsFile, FI_TIME, &tagsTime) == TRUE &&
            ObtainFileInfo(hashfile, FI_TIME, &hashTime) == TRUE &&
            tagsTime > hashTime)
    {
      // recreate the .hsh file if it is outdated
      createHashFile = TRUE;
    }
    else
    {
      createHashFile = FALSE;
    }

    if(createHashFile == TRUE)
      CreateHashTable(C->TagsFile, hashfile, C->TagsSeparator);

    if((fh_tag = fopen(C->TagsFile, "r")) != NULL)
    {
      ULONG hsize;

      if(ObtainFileInfo(hashfile, FI_SIZE, &hsize) == TRUE && hsize > sizeof(ULONG))
      {
        FILE *fh_hash;

        setvbuf(fh_tag, NULL, _IOFBF, SIZE_FILEBUF);

        if((fh_hash = fopen(hashfile, "r")) != NULL)
        {
          ULONG fpos;

          setvbuf(fh_hash, NULL, _IOFBF, SIZE_FILEBUF);

          hsize /= sizeof(ULONG);

          // calculate a random offset
          fpos = (rand() % hsize) * sizeof(ULONG);
          fseek(fh_hash, fpos, SEEK_SET);

          // read the offset to the tagline from the hash file
          if(ReadUInt32(fh_hash, &fpos) == 1)
          {
            char *buf = NULL;
            size_t bufsize = 0;

            fseek(fh_tag, fpos, SEEK_SET);

            if(GetLine(&buf, &bufsize, fh_tag) >= 0)
            {
              fputs(buf, fh_mail);

              while(GetLine(&buf, &bufsize, fh_tag) >= 0)
              {
                if(strncmp(buf, C->TagsSeparator, strlen(C->TagsSeparator)) == 0)
                  break;
                else
                  fprintf(fh_mail, "\n%s", buf);
              }
            }

            free(buf);
          }

          fclose(fh_hash);
        }
      }
      else
        ER_NewError(tr(MSG_ER_CantOpenFile), hashfile);

      fclose(fh_tag);
    }
    else
      ER_NewError(tr(MSG_ER_CantOpenFile), C->TagsFile);
  }

  LEAVE();
}

///
/// WriteSignature
//  Writes signature to an already open file handle
void WriteSignature(FILE *out, struct SignatureNode *sn, BOOL separator)
{
  ENTER();

  if(sn != NULL)
  {
    char *sigText;

    if(sn->useSignatureFile == TRUE)
    {
      char sigPath[SIZE_PATHFILE];

      // read the signature text from the specified file
      sigText = FileToBuffer(CreateFilename(sn->filename, sigPath, sizeof(sigPath)), NULL);
    }
    else
    {
      // use the manually typed signature text
      sigText = sn->signature;
    }

    if(sigText != NULL)
    {
      char ch;
      char *sigPtr = sigText;

      if(separator == TRUE)
        fputs("-- \n", out);
      else
        fputs("\n", out);

      // parse the signature text and replace the %e and %t placeholders
      while((ch = *sigPtr++) != '\0')
      {
        if(ch == '%')
        {
          if(*sigPtr == 't')
          {
            sigPtr++;
            AddTagline(out);
            continue;
          }

          if(*sigPtr == 'e')
          {
            sigPtr++;
            CopyFile(NULL, out, "ENV:SIGNATURE", NULL);
            continue;
          }

          ch = '%';
        }
        fputc(ch, out);
      }
    }

    // free the external signature text string again
    if(sn->useSignatureFile == TRUE)
      free(sigText);
  }

  LEAVE();
}

///
