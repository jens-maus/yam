/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2010 by YAM Open Source Team

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

 Superclass:  MUIC_Window
 Description: Write window class

***************************************************************************/

#include "WriteWindow_cl.h"

#include <string.h>

#include <proto/codesets.h>
#include <proto/dos.h>
#include <proto/wb.h>
#include <dos/notify.h>
#include <workbench/startup.h>

#include "YAM_addressbook.h"
#include "YAM_addressbookEntry.h"
#include "YAM_configGUI.h"
#include "YAM_error.h"
#include "YAM_glossarydisplay.h"
#include "YAM_mainFolder.h"

#include "FileInfo.h"
#include "MailList.h"
#include "MailServers.h"
#include "MUIObjects.h"
#include "Requesters.h"
#include "Threads.h"

#include "mime/uucode.h"
#include "tcp/smtp.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *RG_PAGE;
  Object *ST_TO;
  Object *ST_SUBJECT;
  Object *TX_POSI;
  Object *TE_EDIT;
  Object *TO_TOOLBAR;
  Object *LV_ATTACH;
  Object *BT_ADD;
  Object *BT_ADDPACK;
  Object *BT_DEL;
  Object *BT_DISPLAY;
  Object *RA_ENCODING;
  Object *CY_CTYPE;
  Object *ST_CTYPE;
  Object *ST_DESC;
  Object *ST_CC;
  Object *ST_BCC;
  Object *ST_FROM;
  Object *ST_REPLYTO;
  Object *ST_EXTHEADER;
  Object *CH_DELSEND;
  Object *CH_MDN;
  Object *CH_ADDINFO;
  Object *CY_IMPORTANCE;
  Object *RA_SECURITY;
  Object *CH_DEFSECURITY;
  Object *RA_SIGNATURE;
  Object *BT_HOLD;
  Object *BT_QUEUE;
  Object *BT_SEND;
  Object *BT_CANCEL;
  Object *MI_BOLD;
  Object *MI_ITALIC;
  Object *MI_UNDERLINE;
  Object *MI_COLORED;
  Object *MI_AUTOSPELL;
  Object *MI_AUTOWRAP;
  Object *MI_DELSEND;
  Object *MI_MDN;
  Object *MI_ADDINFO;
  Object *MI_FFONT;
  Object *MI_TCOLOR;
  Object *MI_TSTYLE;
  Object *WI_SEARCH;
  Object *PO_CHARSET;

  struct WriteMailData *wmData; // ptr to write mail data structure
  struct MsgPort *notifyPort;
  struct NotifyRequest *notifyRequest;

  int windowNumber; // the unique window number
  BOOL autoSaved;   // was this mail automatically saved?

  BOOL useFixedFont;   // use a fixed font for displaying the mail
  BOOL useTextColors;  // use Textcolors for displaying the mail
  BOOL useTextStyles;  // use Textstyles for displaying the mail

  char cursorPos[SIZE_SMALL];
  char windowTitle[SIZE_SUBJECT+1]; // string for the title text of the window
};
*/

/* EXPORT
enum RcptType
{
  MUIV_WriteWindow_RcptType_To = 0,
  MUIV_WriteWindow_RcptType_Cc,
  MUIV_WriteWindow_RcptType_BCC,
  MUIV_WriteWindow_RcptType_ReplyTo,
  MUIV_WriteWindow_RcptType_From
};

enum ActiveObject
{
  MUIV_WriteWindow_ActiveObject_To = 0,
  MUIV_WriteWindow_ActiveObject_TextEditor,
  MUIV_WriteWindow_ActiveObject_Subject
};
*/

// menu item IDs
enum
{
  WMEN_NEW=501,WMEN_OPEN,WMEN_INSFILE,WMEN_SAVEAS,WMEN_INSQUOT,WMEN_INSALTQUOT,
  WMEN_INSROT13,WMEN_EDIT,WMEN_CUT,WMEN_COPY,WMEN_PASTE,WMEN_DELETE,WMEN_SELECTALL,
  WMEN_SELECTNONE,WMEN_PASQUOT,WMEN_PASALTQUOT,WMEN_PASROT13,WMEN_SEARCH,WMEN_SEARCHAGAIN,
  WMEN_DICT,WMEN_STYLE_BOLD,WMEN_STYLE_ITALIC,WMEN_STYLE_UNDERLINE,
  WMEN_STYLE_COLORED,WMEN_EMOT0,WMEN_EMOT1,WMEN_EMOT2,WMEN_EMOT3,WMEN_UNDO,WMEN_REDO,
  WMEN_AUTOSP,WMEN_AUTOWRAP,WMEN_ADDFILE, WMEN_ADDCLIP, WMEN_ADDPGP,
  WMEN_DELSEND,WMEN_MDN,WMEN_ADDINFO,WMEN_IMPORT0,WMEN_IMPORT1,
  WMEN_IMPORT2,WMEN_SIGN0,WMEN_SIGN1,WMEN_SIGN2,WMEN_SIGN3,
  WMEN_SECUR0,WMEN_SECUR1,WMEN_SECUR2,WMEN_SECUR3,WMEN_SECUR4, WMEN_SECUR5, WMEN_INSUUCODE,
  WMEN_SENDNOW,WMEN_QUEUE,WMEN_HOLD,WMEN_CANCEL,WMEN_SWITCH1,WMEN_SWITCH2,WMEN_SWITCH3,
  WMEN_FFONT,WMEN_TSTYLE,WMEN_TCOLOR
};

// style origin IDs
enum
{
  ORIGIN_MENU=0,
  ORIGIN_TOOLBAR
};

/* Private Functions */
/// WhichEncodingForFile
//  Determines best MIME encoding mode for a file
static enum Encoding WhichEncodingForFile(const char *fname, const char *ctype)
{
  enum Encoding encoding = ENC_B64;

  ENTER();

  // we make sure that the following content-types get always encoded via base64
  if(strnicmp(ctype, "image/", 6) != 0 &&
     strnicmp(ctype, "audio/", 6) != 0 &&
     strnicmp(ctype, "video/", 6) != 0)
  {
    FILE *fh;

    if((fh = fopen(fname, "r")) != NULL)
    {
      int c;
      int linesize = 0;
      int total = 0;
      int unsafechars = 0;
      int binarychars = 0;
      int longlines = 0;

      setvbuf(fh, NULL, _IOFBF, SIZE_FILEBUF);

      // if there is no special stuff within the file we can break out
      // telling the caller that there is no encoding needed.
      encoding = ENC_7BIT;

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
          if(linesize > 998)
            ++longlines;

          linesize = 0;
        }
        else if (c > 127)
          ++unsafechars; // count the number of unprintable >7bit characters
        else if (c < 32 && c != '\t')
          ++binarychars; // count the number of chars used in binaries.

        // if we successfully scanned 4000 bytes out of the file and found enough
        // data we break out here. we have to at least find some longlines or
        // we have to scan the whole part.
        if(total > 4000 && longlines > 0)
          break;
      }

      fclose(fh);

      D(DBF_MIME, "EncodingTest [%s] t:%ld l:%ld u:%ld b:%ld", fname, total, longlines, unsafechars, binarychars);

      // now that we analyzed the file we have to decide which encoding to take
      if(longlines != 0 || unsafechars != 0 || binarychars != 0)
      {
        if(unsafechars == 0 && binarychars == 0)
        {
          // if we are here just because of long lines we have to use quoted-printable
          // encoding or otherwise we have too long lines in our final mail
          encoding = ENC_QP;
        }
        else if(binarychars == 0 && longlines == 0/* && C->Allow8bit == TRUE */)
        {
          #warning FIXME: Address Allow8bit case for multiple SMTP servers!

          // if there are no binary chars and no long lines in the file and if
          // our SMTP server support 8bit character we can go and encode it via 8bit
          encoding = ENC_8BIT;
        }
        else if(total / (unsafechars+binarychars+1) < 16 || strnicmp(ctype, "application/", 12) == 0)
        {
          // if we end up here we have a file with just unprintable characters
          // and we have to decide if we take base64 or quoted-printable.
          // base64 is more compact if there are many unprintable characters, as
          // when sending a graphics file or such. see (RFC 1521)
          encoding = ENC_B64;
        }
        else
        {
          encoding = ENC_QP;
        }
      }
    }
  }

  D(DBF_MIME, "identified suitable MIME encoding %ld for file [%s]", encoding, fname);

  RETURN(encoding);
  return encoding;
}

///
/// SetDefaultSecurity
static BOOL SetDefaultSecurity(struct Compose *comp)
{
  BOOL result = TRUE;
  enum Security security = SEC_NONE;
  BOOL FirstAddr = TRUE;
  BOOL warnedAlready = FALSE;
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

          // check if PGP is present if security is/was set to sign/encrypt
          if(G->PGPVersion == 0 &&
             (currsec == SEC_SIGN || currsec == SEC_ENCRYPT || currsec == SEC_BOTH))
          {
            if(warnedAlready)
              currsec = SEC_NONE;
            else
            {
              char address[SIZE_LARGE];

              // warn the user about this exeptional situation
              if(MUI_Request(G->App, NULL, 0, tr(MSG_WR_INVALIDSECURITY_TITLE),
                                              tr(MSG_WR_INVALIDSECURITY_GADS),
                                              tr(MSG_WR_INVALIDSECURITY),
                                              BuildAddress(address, sizeof(address), ab->Address, ab->RealName)) != 0)
              {
                currsec = SEC_NONE;
              }
              else
              {
                result = FALSE;
                break;
              }

              warnedAlready = TRUE;
            }
          }
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
            int res = MUI_Request(G->App, NULL, 0, NULL, tr(MSG_WR_SECURITYREQ_GADS),
                                                         tr(MSG_WR_SECURITYREQ));

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
/// BuildPartsList()
//  Builds message parts from attachment list
static struct WritePart *BuildPartsList(struct WriteMailData *wmData)
{
  struct WritePart *first;

  ENTER();

  // create the first MIME part
  if((first = NewMIMEpart(wmData)) != NULL)
  {
    int i;
    struct WritePart *p;

    p = first;
    p->IsTemp = TRUE;
    p->EncType = WhichEncodingForFile(p->Filename, p->ContentType);

    // now walk through our attachment list
    // and create additional parts
    for(i=0; ;i++)
    {
      struct Attach *att = NULL;
      struct WritePart *np = NULL;

      DoMethod(wmData->window, MUIM_WriteWindow_GetAttachment, i, &att);
      if(att == NULL)
        break;

      // we check if the file from the attachment list
      // still exists and is readable
      if(FileExists(att->FilePath) == TRUE)
      {
        // create a new MIME part for the attachment
        if((np = NewMIMEpart(wmData)) != NULL)
        {
          // link the two parts together
          p->Next = np;

          // and now set the information for the new part
          np->ContentType = att->ContentType;
          np->Filename    = att->FilePath;
          np->Description = att->Description;
          np->Name        = att->Name;
          np->IsTemp      = att->IsTemp;

          // find out which encoding we use for the attachment
          if(att->IsMIME == TRUE)
            np->EncType = WhichEncodingForFile(np->Filename, np->ContentType);
          else
            np->EncType = ENC_UUE;

          np->charset = G->writeCharset;

          p = np;
        }
      }
      else
      {
        W(DBF_MAIL, "file from attachmentlist doesn't exist anymore: '%s'");

        ER_NewError(tr(MSG_ER_MISSINGATTFILE), att->FilePath);
      }

      if(np == NULL)
      {
        // an error occurred as we couldn't create a new part
        FreePartsList(first);
        first = NULL;

        break;
      }
    }
  }

  RETURN(first);
  return first;
}

///
/// TransformText()
//  Inserts or pastes text as plain, ROT13, uuencoded or quoted text
static char *TransformText(const char *source, const enum TransformMode mode, const char *qtext)
{
  char *dest = NULL;
  LONG size;

  ENTER();

  if(ObtainFileInfo(source, FI_SIZE, &size) == TRUE && size > 0)
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
/// CreateHashTable()
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
/// AddTagline()
//  Randomly selects a tagline and writes it to the message file
static void AddTagline(FILE *fh_mail)
{
  ENTER();

  if(C->TagsFile[0] != '\0')
  {
    char hashfile[SIZE_PATHFILE];
    LONG tagsTime;
    LONG hashTime;
    FILE *fh_tag;

    snprintf(hashfile, sizeof(hashfile), "%s.hsh", C->TagsFile);

    if(ObtainFileInfo(C->TagsFile, FI_TIME, &tagsTime) == TRUE &&
       ObtainFileInfo(hashfile, FI_TIME, &hashTime) == TRUE &&
       tagsTime > hashTime)
    {
      CreateHashTable(C->TagsFile, hashfile, C->TagsSeparator);
    }

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
          if(ReadUInt32(fh_hash, (APTR)&fpos) == 1)
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
        }

        fclose(fh_hash);
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
/// WriteSignature()
//  Writes signature to the message file
static void WriteSignature(FILE *out, int signat)
{
  char sigPath[SIZE_PATHFILE];
  char *sigFile;
  LONG sigSize;

  ENTER();

  sigFile = CreateFilename(SigNames[signat], sigPath, sizeof(sigPath));

  // check whether the signature file exists and contains at least one character
  if(ObtainFileInfo(sigFile, FI_SIZE, &sigSize) == TRUE && sigSize > 0)
  {
    FILE *in;

    // now append the signature file and fill the placeholders
    if((in = fopen(sigFile, "r")) != NULL)
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
            AddTagline(out);
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
  }

  LEAVE();
}

///

/* Hooks */
/// CloseWriteWindowHook()
//  Hook that will be called as soon as a write window is closed
HOOKPROTONHNO(CloseWriteWindowFunc, void, struct WriteMailData **arg)
{
  struct WriteMailData *wmData = arg[0];

  ENTER();

  // only if this is not a close operation because the application
  // is getting iconified we really cleanup our readmail data
  if(wmData == G->ActiveRexxWMData ||
     xget(G->App, MUIA_Application_Iconified) == FALSE)
  {
    // calls the CleanupWriteMailData to clean everything else up
    CleanupWriteMailData(wmData);
  }

  LEAVE();
}
MakeStaticHook(CloseWriteWindowHook, CloseWriteWindowFunc);

///
/// AppMessageHook()
// Hook to catch the MUIA_AppMessage from a workbench icon drop operation
// Note, that this must be a hook or otherwise the "struct AppMessage"
// is not valid anymore at the time this function is executed.
HOOKPROTONHNO(AppMessageFunc, LONG, ULONG *arg)
{
  // manage the AppMessage
  struct AppMessage *amsg = (struct AppMessage *)arg[0];
  Object *writeWindow = (Object *)arg[1];

  ENTER();

  if(amsg != NULL && writeWindow != NULL)
  {
    int i;

    // let's walk through all arguments in the appMessage
    for(i=0; i < amsg->am_NumArgs; i++)
    {
      struct WBArg *wa = &amsg->am_ArgList[i];
      char buf[SIZE_PATHFILE];

      NameFromLock(wa->wa_Lock, buf, sizeof(buf));

      if(wa->wa_Name != NULL && strlen(wa->wa_Name) > 0)
      {
        AddPart(buf, (char *)wa->wa_Name, sizeof(buf));

        // call WR_App to let it put in the text of the file
        // to the write window
        DoMethod(writeWindow, MUIM_WriteWindow_DroppedFile, buf);
      }
      else
      {
        // open the usual requester, but let it point to the dropped drawer
        DoMethod(writeWindow, MUIM_WriteWindow_RequestAttachment, buf);
      }
    }
  }

  RETURN(0);
  return 0;
}
MakeStaticHook(AppMessageHook, AppMessageFunc);

///

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  ULONG i=0;
  struct Data *data;
  struct Data *tmpData;
  static const char *rtitles[4] = { NULL, NULL, NULL, NULL };
  static const char *encoding[3];
  static const char *security[SEC_MAXDUMMY+1];
  static const char *priority[4];
  static const char *signat[5];

  ENTER();

  // generate a temporarly struct Data to which we store our data and
  // copy it later on
  if((data = tmpData = calloc(1, sizeof(struct Data))) == NULL)
  {
    RETURN(0);
    return 0;
  }

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

  // set some default values
  data->useFixedFont = C->UseFixedFontWrite;
  data->useTextColors = C->UseTextColorsWrite;
  data->useTextStyles = C->UseTextStylesWrite;

  // before we create all objects of this new write window we have to
  // check which number we can set for this window. Therefore we search in our
  // current WriteMailData list and check which number we can give this window
  do
  {
    struct Node *curNode;
    BOOL found = FALSE;

    IterateList(&G->writeMailDataList, curNode)
    {
      struct WriteMailData *wmData = (struct WriteMailData *)curNode;

      if(wmData->window != NULL &&
         xget(wmData->window, MUIA_WriteWindow_Num) == i)
      {
        found = TRUE;
        break;
      }
    }

    // if we didn't find a window with the current ID then we can choose it as
    // our WriteWindow ID
    if(found == FALSE)
    {
      D(DBF_GUI, "Free write window number %ld found.", i);
      data->windowNumber = i;

      break;
    }

    i++;
  }
  while(TRUE);

  // allocate the new writeMailData structure
  if((data->wmData = AllocWriteMailData()) != NULL)
  {
    Object *menuStripObject = NULL;
    Object *charsetPopButton;
    Object *slider;
    struct TagItem *tags = inittags(msg);
    struct TagItem *tag;

    // check for some tags present at OM_NEW
    while((tag = NextTagItem((APTR)&tags)) != NULL)
    {
      switch(tag->ti_Tag)
      {
        case ATTR(Mode): data->wmData->mode = (enum NewMailMode)tag->ti_Data; break;
        case ATTR(Quiet): data->wmData->quietMode = (BOOL)tag->ti_Data; break;
      }
    }

    // if this write window is processed in NMM_BOUNCE mode we create
    // a slightly different graphical interface and hide all other things
    if(data->wmData->mode == NMM_BOUNCE)
    {
      // create a minimal menu strip for the bounce window which just consists
      // of the standard editing operations and shortcuts for the 4 buttons
      // in this window. Without this menu strip copying and pasteing would be
      // impossible, as the BetterString objects do NOT handle key presses
      // themselves anymore.
      // A list of all currently used shortcuts follows below for the full
      // menu.
      menuStripObject = MenustripObject,
        MenuChild, MenuObject,
          MUIA_Menu_Title, tr(MSG_WR_Text),
          MenuChild, Menuitem(tr(MSG_WR_MSENDNOW), "S", TRUE, FALSE, WMEN_SENDNOW),
          MenuChild, Menuitem(tr(MSG_WR_MSENDLATER), "L", TRUE, FALSE, WMEN_QUEUE),
          MenuChild, Menuitem(tr(MSG_WR_MHOLD), "H", TRUE, FALSE, WMEN_HOLD),
          MenuChild, Menuitem(tr(MSG_WR_MCANCEL), "W", TRUE, FALSE, WMEN_CANCEL),
        End,
        MenuChild, MenuObject,
          MUIA_Menu_Title, tr(MSG_WR_Edit),
          MenuChild, Menuitem(tr(MSG_WR_MUndo), "Z", TRUE, FALSE, WMEN_UNDO),
          MenuChild, Menuitem(tr(MSG_WR_Redo), "Y", TRUE, FALSE, WMEN_REDO),
          MenuChild, MenuBarLabel,
          MenuChild, Menuitem(tr(MSG_WR_MCut), "X", TRUE, FALSE, WMEN_CUT),
          MenuChild, Menuitem(tr(MSG_WR_MCopy), "C", TRUE, FALSE, WMEN_COPY),
          MenuChild, Menuitem(tr(MSG_WR_MPaste), "V", TRUE, FALSE, WMEN_PASTE),
          MenuChild, Menuitem(tr(MSG_WR_DELETE), NULL, TRUE, FALSE, WMEN_DELETE),
          MenuChild, MenuBarLabel,
          MenuChild, Menuitem(tr(MSG_WR_SELECTALL), "A", TRUE, FALSE, WMEN_SELECTALL),
          MenuChild, Menuitem(tr(MSG_WR_SELECTNONE), NULL, TRUE, FALSE, WMEN_SELECTNONE),
        End,
      End;

      if((obj = DoSuperNew(cl, obj,
        MUIA_Window_Title, tr(MSG_WR_BounceWT),
        MUIA_HelpNode, "WR_W",
        MUIA_Window_ID, MAKE_ID('W','R','I','B'),
        MUIA_Window_AppWindow, FALSE,
        MUIA_Window_Menustrip, menuStripObject,
        WindowContents, VGroup,
          Child, ColGroup(2),
            Child, Label2(tr(MSG_WR_BounceTo)),
            Child, MakeAddressField(&data->ST_TO, tr(MSG_WR_BounceTo), MSG_HELP_WR_ST_TO, ABM_TO, data->windowNumber, AFF_ALLOW_MULTI|AFF_EXTERNAL_SHORTCUTS),
          End,
          Child, ColGroup(4),
            Child, data->BT_SEND   = MakeButton(tr(MSG_WR_SENDNOW)),
            Child, data->BT_QUEUE  = MakeButton(tr(MSG_WR_SENDLATER)),
            Child, data->BT_HOLD   = MakeButton(tr(MSG_WR_HOLD)),
            Child, data->BT_CANCEL = MakeButton(tr(MSG_WR_CANCEL)),
          End,
        End,
      TAG_MORE, inittags(msg))) != NULL)
      {
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_SENDNOW,    obj, 2, MUIM_WriteWindow_ComposeMail, WRITE_SEND);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_QUEUE,      obj, 2, MUIM_WriteWindow_ComposeMail, WRITE_QUEUE);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_HOLD,       obj, 2, MUIM_WriteWindow_ComposeMail, WRITE_HOLD);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_CANCEL,     obj, 1, MUIM_WriteWindow_CancelAction);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_CUT,        obj, 2, MUIM_WriteWindow_EditActionPerformed, EA_CUT);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_COPY,       obj, 2, MUIM_WriteWindow_EditActionPerformed, EA_COPY);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_PASTE,      obj, 2, MUIM_WriteWindow_EditActionPerformed, EA_PASTE);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_DELETE,     obj, 2, MUIM_WriteWindow_EditActionPerformed, EA_DELETE);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_SELECTALL,  obj, 2, MUIM_WriteWindow_EditActionPerformed, EA_SELECTALL);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_SELECTNONE, obj, 2, MUIM_WriteWindow_EditActionPerformed, EA_SELECTNONE);
      }
    }
    else
    {
      // now we create the Menustrip object with all the menu items
      // and corresponding shortcuts
      //
      // The follwong shortcut list should help to identify the hard-coded
      // shortcuts:
      //
      //  A   reserved for 'Select All' operation (WMEN_SELECTALL)
      //  B   Bold soft-style (WMEN_STYLE_BOLD)
      //  C   reserved for 'Copy' operation (WMEN_COPY)
      //  D   Dictonary (WMEN_DICT)
      //  E   Launch editor (WMEN_EDIT)
      //  F   Find/Search (WMEN_SEARCH)
      //  G   Search again (WMEN_SEARCHAGAIN)
      //  H   Hold mail (reserved by YAM.cd)
      //  I   Italic soft-style (WMEN_STYLE_BOLD)
      //  J
      //  K   Colored soft-style (WMEN_STYLE_COLORED)
      //  L   Send later (reserved by YAM.cd)
      //  M
      //  N   New mail (WMEN_NEW)
      //  O   Open file (WMEN_OPEN)
      //  P   Insert as plain text (WMEN_PLAIN)
      //  Q   Insert as quoted text (WMEN_PASQUOT)
      //  R   Add file as attachment (WMEN_ADDFILE)
      //  S   Send mail (WMEN_SEND)
      //  T   Enable/Disable Text Colors (RMEN_TCOLOR)
      //  U   Underline soft-style (WMEN_STYLE_UNDERLINE)
      //  V   reserved for 'Paste' operation (WMEN_PASTE)
      //  W   Cancel&Close window (WMEN_CANCEL)
      //  X   reserved for 'Cut' operation (WMEN_CUT)
      //  Y   reserved for 'Redo' operation (WMEN_REDO)
      //  Z   reserved for 'Undo' operation (WMEN_UNDO)
      //  1   Switch to Message view (WMEN_SWITCH1)
      //  2   Switch to Attachment view (WMEN_SWITCH2)
      //  3   Switch to Options view (WMEN_SWITCH3)
      //  0   Use signature1 (WMEN_SIGN0)
      //  7   Use signature2 (WMEN_SIGN1)
      //  8   Use signature3 (WMEN_SIGN2)
      //  9   Use signature4 (WMEN_SIGN3)

      menuStripObject = MenustripObject,
        MenuChild, MenuObject,
          MUIA_Menu_Title, tr(MSG_WR_Text),
          MenuChild, Menuitem(tr(MSG_New), "N", TRUE, FALSE, WMEN_NEW),
          MenuChild, Menuitem(tr(MSG_Open), "O", TRUE, FALSE, WMEN_OPEN),
          MenuChild, MenuitemObject,
            MUIA_Menuitem_Title, tr(MSG_WR_InsertAs),
            MenuChild, Menuitem(tr(MSG_WR_Plain), "P", TRUE, FALSE, WMEN_INSFILE),
            MenuChild, Menuitem(tr(MSG_WR_Quoted), NULL, TRUE, FALSE, WMEN_INSQUOT),
            MenuChild, Menuitem(tr(MSG_WR_AltQuoted), NULL, TRUE, FALSE, WMEN_INSALTQUOT),
            MenuChild, Menuitem(tr(MSG_WR_ROT13), NULL, TRUE, FALSE, WMEN_INSROT13),
            MenuChild, Menuitem(tr(MSG_WR_UUCODE), NULL, TRUE, FALSE, WMEN_INSUUCODE),
          End,
          MenuChild, MenuBarLabel,
          MenuChild, Menuitem(tr(MSG_WR_LaunchEd), "E", C->Editor[0] != '\0', FALSE, WMEN_EDIT),
          MenuChild, MenuBarLabel,
          MenuChild, Menuitem(tr(MSG_SaveAs), NULL, TRUE, FALSE, WMEN_SAVEAS),
          MenuChild, MenuBarLabel,
          MenuChild, Menuitem(tr(MSG_WR_MSENDNOW), "S", TRUE, FALSE, WMEN_SENDNOW),
          MenuChild, Menuitem(tr(MSG_WR_MSENDLATER), "L", TRUE, FALSE, WMEN_QUEUE),
          MenuChild, Menuitem(tr(MSG_WR_MHOLD), "H", TRUE, FALSE, WMEN_HOLD),
          MenuChild, Menuitem(tr(MSG_WR_MCANCEL), "W", TRUE, FALSE, WMEN_CANCEL),
        End,
        MenuChild, MenuObject,
          MUIA_Menu_Title, tr(MSG_WR_Edit),
          MenuChild, Menuitem(tr(MSG_WR_MUndo), "Z", TRUE, FALSE, WMEN_UNDO),
          MenuChild, Menuitem(tr(MSG_WR_Redo), "Y", TRUE, FALSE, WMEN_REDO),
          MenuChild, MenuBarLabel,
          MenuChild, Menuitem(tr(MSG_WR_MCut), "X", TRUE, FALSE, WMEN_CUT),
          MenuChild, Menuitem(tr(MSG_WR_MCopy), "C", TRUE, FALSE, WMEN_COPY),
          MenuChild, Menuitem(tr(MSG_WR_MPaste), "V", TRUE, FALSE, WMEN_PASTE),
          MenuChild, MenuitemObject,
            MUIA_Menuitem_Title, tr(MSG_WR_PasteAs),
            MenuChild, Menuitem(tr(MSG_WR_Quoted), "Q", TRUE, FALSE, WMEN_PASQUOT),
            MenuChild, Menuitem(tr(MSG_WR_AltQuoted), NULL, TRUE, FALSE, WMEN_PASALTQUOT),
            MenuChild, Menuitem(tr(MSG_WR_ROT13), NULL, TRUE, FALSE, WMEN_PASROT13),
          End,
          MenuChild, Menuitem(tr(MSG_WR_DELETE), NULL, TRUE, FALSE, WMEN_DELETE),
          MenuChild, MenuBarLabel,
          MenuChild, Menuitem(tr(MSG_WR_SELECTALL), "A", TRUE, FALSE, WMEN_SELECTALL),
          MenuChild, Menuitem(tr(MSG_WR_SELECTNONE), NULL, TRUE, FALSE, WMEN_SELECTNONE),
          MenuChild, MenuBarLabel,
          MenuChild, Menuitem(tr(MSG_WR_SEARCH), "F", TRUE, FALSE, WMEN_SEARCH),
          MenuChild, Menuitem(tr(MSG_WR_SEARCH_AGAIN), "G", TRUE, FALSE, WMEN_SEARCHAGAIN),
          MenuChild, MenuBarLabel,
          MenuChild, Menuitem(tr(MSG_WR_Dictionary), "D", TRUE, FALSE, WMEN_DICT),
          MenuChild, MenuitemObject,
            MUIA_Menuitem_Title,tr(MSG_WR_Textstyle),
            MenuChild, data->MI_BOLD = MenuitemCheck(tr(MSG_WR_Bold), "B", TRUE, FALSE, TRUE, 0, WMEN_STYLE_BOLD),
            MenuChild, data->MI_ITALIC = MenuitemCheck(tr(MSG_WR_Italic), "I", TRUE, FALSE, TRUE, 0, WMEN_STYLE_ITALIC),
            MenuChild, data->MI_UNDERLINE = MenuitemCheck(tr(MSG_WR_Underlined), "U", TRUE, FALSE, TRUE, 0, WMEN_STYLE_UNDERLINE),
            MenuChild, data->MI_COLORED = MenuitemCheck(tr(MSG_WR_Colored), "K", TRUE, FALSE, TRUE, 0, WMEN_STYLE_COLORED),
          End,
          MenuChild, MenuitemObject,
            MUIA_Menuitem_Title,tr(MSG_WR_Emoticons),
            MenuChild, Menuitem(tr(MSG_WR_Happy), NULL, TRUE, FALSE, WMEN_EMOT0),
            MenuChild, Menuitem(tr(MSG_WR_Indifferent), NULL, TRUE, FALSE, WMEN_EMOT1),
            MenuChild, Menuitem(tr(MSG_WR_Sad), NULL, TRUE, FALSE, WMEN_EMOT2),
            MenuChild, Menuitem(tr(MSG_WR_Ironic), NULL, TRUE, FALSE, WMEN_EMOT3),
          End,
          MenuChild, MenuBarLabel,
          MenuChild, data->MI_AUTOSPELL = MenuitemCheck(tr(MSG_WR_SpellCheck), NULL, TRUE, FALSE, TRUE, 0, WMEN_AUTOSP),
          MenuChild, data->MI_AUTOWRAP = MenuitemCheck(tr(MSG_WR_AUTOWRAP), NULL, TRUE, FALSE, TRUE, 0, WMEN_AUTOWRAP),
        End,
        MenuChild, MenuObject,
          MUIA_Menu_Title, tr(MSG_Attachments),
          MenuChild, Menuitem(tr(MSG_WR_MAddFile), "R", TRUE, FALSE, WMEN_ADDFILE),
          MenuChild, Menuitem(tr(MSG_WR_AddCB), NULL, TRUE, FALSE, WMEN_ADDCLIP),
          MenuChild, Menuitem(tr(MSG_WR_AddKey), NULL, G->PGPVersion != 0, FALSE, WMEN_ADDPGP),
        End,
        MenuChild, MenuObject,
          MUIA_Menu_Title, tr(MSG_WR_VIEW),
          MenuChild, Menuitem(tr(MSG_WR_MSWITCH_MSG), "1", TRUE, FALSE, WMEN_SWITCH1),
          MenuChild, Menuitem(tr(MSG_WR_MSWITCH_ATT), "2", TRUE, FALSE, WMEN_SWITCH2),
          MenuChild, Menuitem(tr(MSG_WR_MSWITCH_OPT), "3", TRUE, FALSE, WMEN_SWITCH3),
        End,
        MenuChild, MenuObject,
          MUIA_Menu_Title, tr(MSG_Options),
          MenuChild, data->MI_DELSEND = MenuitemCheck(tr(MSG_WR_MDelSend), NULL, TRUE, FALSE, TRUE, 0, WMEN_DELSEND),
          MenuChild, data->MI_MDN = MenuitemCheck(tr(MSG_WR_MReceipt), NULL, TRUE, FALSE, TRUE, 0, WMEN_MDN),
          MenuChild, data->MI_ADDINFO = MenuitemCheck(tr(MSG_WR_MAddInfo), NULL, TRUE, FALSE, TRUE, 0, WMEN_ADDINFO),
          MenuChild, MenuitemObject,
            MUIA_Menuitem_Title,tr(MSG_WR_MImportance),
            MenuChild, MenuitemCheck(priority[0], NULL, TRUE, FALSE, TRUE, 0x06, WMEN_IMPORT0),
            MenuChild, MenuitemCheck(priority[1], NULL, TRUE, TRUE,  TRUE, 0x05, WMEN_IMPORT1),
            MenuChild, MenuitemCheck(priority[2], NULL, TRUE, FALSE, TRUE, 0x03, WMEN_IMPORT2),
          End,
          MenuChild, MenuitemObject,
            MUIA_Menuitem_Title,tr(MSG_CO_CrdSignature),
            MenuChild, MenuitemCheck(signat[0], "0", TRUE, C->UseSignature == FALSE, TRUE, 0x0E, WMEN_SIGN0),
            MenuChild, MenuitemCheck(signat[1], "7", TRUE, C->UseSignature == TRUE,  TRUE, 0x0D, WMEN_SIGN1),
            MenuChild, MenuitemCheck(signat[2], "8", TRUE, FALSE, TRUE, 0x0B, WMEN_SIGN2),
            MenuChild, MenuitemCheck(signat[3], "9", TRUE, FALSE, TRUE, 0x07, WMEN_SIGN3),
          End,
          MenuChild, MenuitemObject,
            MUIA_Menuitem_Title,tr(MSG_CO_CrdSecurity),
            MenuChild, MenuitemCheck(security[SEC_NONE], NULL, TRUE, FALSE, TRUE, 0x3E, WMEN_SECUR0),
            MenuChild, MenuitemCheck(security[SEC_SIGN], NULL, G->PGPVersion != 0, FALSE, TRUE, 0x3D, WMEN_SECUR1),
            MenuChild, MenuitemCheck(security[SEC_ENCRYPT], NULL, G->PGPVersion != 0, FALSE, TRUE, 0x3B, WMEN_SECUR2),
            MenuChild, MenuitemCheck(security[SEC_BOTH], NULL, G->PGPVersion != 0, FALSE, TRUE, 0x37, WMEN_SECUR3),
            MenuChild, MenuitemCheck(security[SEC_SENDANON], NULL, TRUE, FALSE, TRUE, 0x2F, WMEN_SECUR4),
            MenuChild, MenuitemCheck(security[SEC_DEFAULTS], NULL, TRUE, TRUE, TRUE, 0x1F, WMEN_SECUR5),
          End,
          MenuChild, MenuBarLabel,
          MenuChild, data->MI_FFONT  = MenuitemCheck(tr(MSG_WR_FIXEDFONT),  NULL, TRUE, data->useFixedFont,  TRUE, 0, WMEN_FFONT),
          MenuChild, data->MI_TCOLOR = MenuitemCheck(tr(MSG_WR_TEXTCOLORS), "T",  TRUE, data->useTextColors, TRUE, 0, WMEN_TCOLOR),
          MenuChild, data->MI_TSTYLE = MenuitemCheck(tr(MSG_WR_TEXTSTYLES), NULL, TRUE, data->useTextStyles, TRUE, 0, WMEN_TSTYLE),
        End,
      End;

      // create the slider for the text editor
      slider = ScrollbarObject, End;

      // create the write window object
      if(menuStripObject != NULL && slider != NULL)
      {
        obj = DoSuperNew(cl, obj,

          MUIA_Window_Title, "",
          MUIA_HelpNode, "WR_W",
          MUIA_Window_ID, MAKE_ID('W','R','W', data->windowNumber),
          MUIA_Window_AppWindow, TRUE,
          MUIA_Window_Menustrip, menuStripObject,
          WindowContents, VGroup,
            Child, data->RG_PAGE = RegisterGroup(rtitles),
              MUIA_CycleChain, TRUE,

              // Message
              Child, VGroup,
                MUIA_HelpNode, "WR00",

                Child, ColGroup(2),
                  Child, Label(tr(MSG_WR_To)),
                  Child, MakeAddressField(&data->ST_TO, tr(MSG_WR_To), MSG_HELP_WR_ST_TO, ABM_TO, data->windowNumber, AFF_ALLOW_MULTI|AFF_EXTERNAL_SHORTCUTS),

                  Child, Label(tr(MSG_WR_Subject)),
                  Child, data->ST_SUBJECT = BetterStringObject,
                    StringFrame,
                    MUIA_BetterString_NoShortcuts, TRUE,
                    MUIA_String_MaxLen,            SIZE_SUBJECT,
                    MUIA_String_AdvanceOnCR,       TRUE,
                    MUIA_ControlChar,              ShortCut(tr(MSG_WR_Subject)),
                    MUIA_CycleChain,               TRUE,
                  End,
                End,

                Child, hasHideToolBarFlag(C->HideGUIElements) ?
                  (RectangleObject, MUIA_ShowMe, FALSE, End) :
                  (HGroup, GroupSpacing(0),
                    Child, HGroupV,
                      Child, data->TO_TOOLBAR = WriteWindowToolbarObject,
                      End,
                    End,

                    Child, hasHideXYFlag(C->HideGUIElements) ?
                      (HSpace(1)) :
                      (HGroup, GroupSpacing(0),
                        Child, RectangleObject,
                          MUIA_Rectangle_VBar, TRUE,
                          MUIA_FixWidth,       3,
                        End,
                        Child, VGroup,
                          GroupSpacing(0),
                          Child, VSpace(0),
                          Child, data->TX_POSI = TextObject,
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
                  Child, data->TE_EDIT = MailTextEditObject,
                    InputListFrame,
                    MUIA_CycleChain, TRUE,
                    MUIA_TextEditor_Slider,     slider,
                    MUIA_TextEditor_FixedFont,  C->UseFixedFontWrite,
                    MUIA_TextEditor_WrapMode,   MUIV_TextEditor_WrapMode_SoftWrap,
                    MUIA_TextEditor_WrapBorder, C->EdWrapMode == EWM_EDITING ? C->EdWrapCol : 0,
                    MUIA_TextEditor_ExportWrap, C->EdWrapMode != EWM_OFF ? C->EdWrapCol : 0,
                    MUIA_TextEditor_ImportHook, MUIV_TextEditor_ImportHook_Plain,
                    MUIA_TextEditor_ExportHook, MUIV_TextEditor_ExportHook_NoStyle,
                  End,
                  Child, slider,
                End,
              End,

              // Attachments
              Child, VGroup,
                MUIA_HelpNode, "WR01",
                Child, NListviewObject,
                  MUIA_CycleChain, TRUE,
                  MUIA_NListview_NList, data->LV_ATTACH = WriteAttachmentListObject,
                  End,
                End,

                Child, ColGroup(4),
                  Child, data->BT_ADD     = MakeButton(tr(MSG_WR_Add)),
                  Child, data->BT_ADDPACK = MakeButton(tr(MSG_WR_AddPack)),
                  Child, data->BT_DEL     = MakeButton(tr(MSG_Del)),
                  Child, data->BT_DISPLAY = MakeButton(tr(MSG_WR_Display)),
                End,

                Child, HGroup,
                  Child, data->RA_ENCODING = RadioObject,
                    GroupFrameT(tr(MSG_WR_Encoding)),
                    MUIA_Radio_Entries, encoding,
                    MUIA_CycleChain, TRUE,
                  End,
                  Child, ColGroup(2),
                    Child, Label2(tr(MSG_WR_ContentType)),
                    Child, MakeMimeTypePop(&data->ST_CTYPE, tr(MSG_WR_ContentType)),
                    Child, Label2(tr(MSG_WR_Description)),
                    Child, data->ST_DESC = BetterStringObject,
                      StringFrame,
                      MUIA_BetterString_NoShortcuts, TRUE,
                      MUIA_String_MaxLen,            SIZE_DEFAULT,
                      MUIA_String_AdvanceOnCR,       TRUE,
                      MUIA_ControlChar,              ShortCut(tr(MSG_WR_Description)),
                      MUIA_CycleChain,               TRUE,
                    End,
                    Child, HSpace(0),
                  End,
                End,
              End,

              // Options
              Child, VGroup,
                MUIA_HelpNode, "WR02",
                Child, ColGroup(2),
                  Child, Label(tr(MSG_WR_CopyTo)),
                  Child, MakeAddressField(&data->ST_CC, tr(MSG_WR_CopyTo), MSG_HELP_WR_ST_CC, ABM_CC, data->windowNumber, AFF_ALLOW_MULTI|AFF_EXTERNAL_SHORTCUTS),

                  Child, Label(tr(MSG_WR_BlindCopyTo)),
                  Child, MakeAddressField(&data->ST_BCC, tr(MSG_WR_BlindCopyTo), MSG_HELP_WR_ST_BCC, ABM_BCC, data->windowNumber, AFF_ALLOW_MULTI|AFF_EXTERNAL_SHORTCUTS),

                  Child, Label(tr(MSG_WR_From)),
                  Child, MakeAddressField(&data->ST_FROM, tr(MSG_WR_From), MSG_HELP_WR_ST_FROM, ABM_FROM, data->windowNumber, AFF_ALLOW_MULTI|AFF_EXTERNAL_SHORTCUTS),

                  Child, Label(tr(MSG_WR_ReplyTo)),
                  Child, MakeAddressField(&data->ST_REPLYTO, tr(MSG_WR_ReplyTo), MSG_HELP_WR_ST_REPLYTO, ABM_REPLYTO, data->windowNumber, AFF_ALLOW_MULTI|AFF_EXTERNAL_SHORTCUTS),

                  Child, Label(tr(MSG_WR_ExtraHeaders)),
                  Child, data->ST_EXTHEADER = BetterStringObject,
                    StringFrame,
                    MUIA_BetterString_NoShortcuts, TRUE,
                    MUIA_String_MaxLen,            SIZE_LARGE,
                    MUIA_String_AdvanceOnCR,       TRUE,
                    MUIA_ControlChar,              ShortCut(tr(MSG_WR_ExtraHeaders)),
                    MUIA_CycleChain,               TRUE,
                  End,

                  Child, Label(tr(MSG_WR_CHARSET)),
                  Child, MakeCharsetPop((Object **)&data->PO_CHARSET, &charsetPopButton),

                End,

                Child, HGroup,
                  Child, VGroup, GroupFrameT(tr(MSG_WR_SendOpt)),
                    Child, MakeCheckGroup((Object **)&data->CH_DELSEND, tr(MSG_WR_DelSend)),
                    Child, MakeCheckGroup((Object **)&data->CH_MDN,     tr(MSG_WR_Receipt)),
                    Child, MakeCheckGroup((Object **)&data->CH_ADDINFO, tr(MSG_WR_AddInfo)),
                    Child, HGroup,
                      Child, Label(tr(MSG_WR_Importance)),
                      Child, data->CY_IMPORTANCE = MakeCycle(priority, tr(MSG_WR_Importance)),
                    End,
                  End,

                  Child, HSpace(0),
                  Child, data->RA_SIGNATURE = RadioObject,
                    GroupFrameT(tr(MSG_WR_Signature)),
                    MUIA_CycleChain,    TRUE,
                    MUIA_Radio_Entries, signat,
                    MUIA_Radio_Active,  C->UseSignature ? 1 : 0,
                  End,
                  Child, HSpace(0),
                  Child, data->RA_SECURITY = RadioObject,
                    GroupFrameT(tr(MSG_WR_Security)),
                    MUIA_CycleChain,    TRUE,
                    MUIA_Radio_Entries, security,
                    MUIA_Radio_Active,  SEC_DEFAULTS,
                  End,
                End,
              End,

            End,

            // Buttons
            Child, ColGroup(4),
              Child, data->BT_SEND   = MakeButton(tr(MSG_WR_SENDNOW)),
              Child, data->BT_QUEUE  = MakeButton(tr(MSG_WR_SENDLATER)),
              Child, data->BT_HOLD   = MakeButton(tr(MSG_WR_HOLD)),
              Child, data->BT_CANCEL = MakeButton(tr(MSG_WR_CANCEL)),
            End,
          End,

        TAG_MORE, inittags(msg));
      }
    }

    // check if object creation worked as expected
    if(obj != NULL)
    {
      char address[SIZE_LARGE];
      char filename[SIZE_PATHFILE];

      if((data = (struct Data *)INST_DATA(cl,obj)) == NULL)
      {
        FreeWriteMailData(tmpData->wmData);
        free(tmpData);
        RETURN(0);
        return 0;
      }

      // copy back the data stored in our temporarly struct Data
      memcpy(data, tmpData, sizeof(struct Data));

      // place this newly created window to the writeMailData structure aswell
      data->wmData->window = obj;

      // Add the window to the application object
      DoMethod(G->App, OM_ADDMEMBER, obj);

      // set notifies and default options in case this
      // write window is not in BOUNCE mode
      if(data->wmData->mode != NMM_BOUNCE)
      {
        // prepare a first initial window title string
        snprintf(data->windowTitle, sizeof(data->windowTitle), "[%d] %s: ", data->windowNumber+1, tr(MSG_WR_WriteWT));

        // set the default window object
        xset(obj, MUIA_Window_DefaultObject, data->ST_TO,
                  MUIA_Window_ActiveObject,  data->ST_TO,
                  MUIA_Window_Title,         data->windowTitle);

        // set the key focus attributes of the TO and SUBJECT gadgets
        xset(data->ST_TO, MUIA_BetterString_KeyDownFocus, data->ST_SUBJECT,
                          MUIA_Recipientstring_FromString, data->ST_FROM,
                          MUIA_Recipientstring_ReplyToString, data->ST_REPLYTO);

        xset(data->ST_SUBJECT, MUIA_BetterString_KeyUpFocus, data->ST_TO,
                               MUIA_BetterString_KeyDownFocus, data->TE_EDIT);

        // set certain default check settings of the autospell and autowrap menu items
        set(data->MI_AUTOSPELL, MUIA_Menuitem_Checked, xget(data->TE_EDIT, MUIA_TextEditor_TypeAndSpell));
        set(data->MI_AUTOWRAP,  MUIA_Menuitem_Checked, xget(data->TE_EDIT, MUIA_TextEditor_WrapBorder) > 0);

        // set the charset popupbutton string list contents
        set(charsetPopButton, MUIA_ControlChar, ShortCut(tr(MSG_WR_CHARSET)));
        nnset(data->PO_CHARSET, MUIA_Text_Contents, C->DefaultWriteCharset);

        // put the importance cycle gadget into the cycle group
        set(data->CY_IMPORTANCE, MUIA_Cycle_Active, TRUE);

        // disable certain GUI elements per default
        DoMethod(G->App, MUIM_MultiSet,  MUIA_Disabled, TRUE, data->RA_ENCODING,
                                                              data->ST_CTYPE,
                                                              data->ST_DESC,
                                                              data->BT_DEL,
                                                              data->BT_DISPLAY,
                                                              NULL);

        // set the help elements of our GUI gadgets
        SetHelp(data->ST_SUBJECT,   MSG_HELP_WR_ST_SUBJECT);
        SetHelp(data->BT_ADD,       MSG_HELP_WR_BT_ADD);
        SetHelp(data->BT_ADDPACK,   MSG_HELP_WR_BT_ADDPACK);
        SetHelp(data->BT_DEL,       MSG_HELP_WR_BT_DEL);
        SetHelp(data->BT_DISPLAY,   MSG_HELP_WR_BT_DISPLAY);
        SetHelp(data->RA_ENCODING,  MSG_HELP_WR_RA_ENCODING);
        SetHelp(data->ST_CTYPE,     MSG_HELP_WR_ST_CTYPE);
        SetHelp(data->ST_DESC,      MSG_HELP_WR_ST_DESC);
        SetHelp(data->ST_EXTHEADER, MSG_HELP_WR_ST_EXTHEADER);
        SetHelp(data->CH_DELSEND,   MSG_HELP_WR_CH_DELSEND);
        SetHelp(data->CH_MDN,       MSG_HELP_WR_CH_RECEIPT);
        SetHelp(data->CH_ADDINFO,   MSG_HELP_WR_CH_ADDINFO);
        SetHelp(data->CY_IMPORTANCE,MSG_HELP_WR_CY_IMPORTANCE);
        SetHelp(data->RA_SIGNATURE, MSG_HELP_WR_RA_SIGNATURE);
        SetHelp(data->RA_SECURITY,  MSG_HELP_WR_RA_SECURITY);
        SetHelp(data->LV_ATTACH,    MSG_HELP_WR_LV_ATTACH);

        // set the menuitem notifies
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_NEW,        data->TE_EDIT, 1, MUIM_TextEditor_ClearText);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_OPEN,       obj, 2, MUIM_WriteWindow_EditorCmd, ED_OPEN);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_INSFILE,    obj, 2, MUIM_WriteWindow_EditorCmd, ED_INSERT);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_INSQUOT,    obj, 2, MUIM_WriteWindow_EditorCmd, ED_INSQUOT);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_INSALTQUOT, obj, 2, MUIM_WriteWindow_EditorCmd, ED_INSALTQUOT);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_INSROT13,   obj, 2, MUIM_WriteWindow_EditorCmd, ED_INSROT13);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_INSUUCODE,  obj, 2, MUIM_WriteWindow_EditorCmd, ED_INSUUCODE);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_SAVEAS,     obj, 1, MUIM_WriteWindow_SaveTextAs);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_EDIT,       obj, 1, MUIM_WriteWindow_LaunchEditor);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_SENDNOW,    obj, 2, MUIM_WriteWindow_ComposeMail, WRITE_SEND);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_QUEUE,      obj, 2, MUIM_WriteWindow_ComposeMail, WRITE_QUEUE);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_HOLD,       obj, 2, MUIM_WriteWindow_ComposeMail, WRITE_HOLD);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_CANCEL,     obj, 1, MUIM_WriteWindow_CancelAction);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_CUT,        obj, 2, MUIM_WriteWindow_EditActionPerformed, EA_CUT);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_COPY,       obj, 2, MUIM_WriteWindow_EditActionPerformed, EA_COPY);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_PASTE,      obj, 2, MUIM_WriteWindow_EditActionPerformed, EA_PASTE);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_DELETE,     obj, 2, MUIM_WriteWindow_EditActionPerformed, EA_DELETE);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_PASQUOT,    obj, 2, MUIM_WriteWindow_EditorCmd, ED_PASQUOT);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_PASALTQUOT, obj, 2, MUIM_WriteWindow_EditorCmd, ED_PASALTQUOT);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_PASROT13,   obj, 2, MUIM_WriteWindow_EditorCmd, ED_PASROT13);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_SEARCH,     obj, 2, MUIM_WriteWindow_Search, MUIF_NONE);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_SEARCHAGAIN,obj, 2, MUIM_WriteWindow_Search, MUIF_ReadMailGroup_Search_Again);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_DICT,       MUIV_Notify_Application, 3, MUIM_CallHook, &DI_OpenHook, obj);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_UNDO,       obj, 2, MUIM_WriteWindow_EditActionPerformed, EA_UNDO);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_REDO,       obj, 2, MUIM_WriteWindow_EditActionPerformed, EA_REDO);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_ADDFILE,    obj, 2, MUIM_WriteWindow_RequestAttachment, C->AttachDir);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_ADDCLIP,    obj, 1, MUIM_WriteWindow_AddClipboard);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_ADDPGP,     obj, 1, MUIM_WriteWindow_AddPGPKey);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_SELECTALL,  obj, 2, MUIM_WriteWindow_EditActionPerformed, EA_SELECTALL);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_SELECTNONE, obj, 2, MUIM_WriteWindow_EditActionPerformed, EA_SELECTNONE);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_SWITCH1,    data->RG_PAGE, 3, MUIM_Set, MUIA_Group_ActivePage, 0);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_SWITCH2,    data->RG_PAGE, 3, MUIM_Set, MUIA_Group_ActivePage, 1);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_SWITCH3,    data->RG_PAGE, 3, MUIM_Set, MUIA_Group_ActivePage, 2);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_EMOT0,      data->TE_EDIT, 2, MUIM_TextEditor_InsertText, ":-)");
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_EMOT1,      data->TE_EDIT, 2, MUIM_TextEditor_InsertText, ":-|");
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_EMOT2,      data->TE_EDIT, 2, MUIM_TextEditor_InsertText, ":-(");
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_EMOT3,      data->TE_EDIT, 2, MUIM_TextEditor_InsertText, ";-)");

        // catch MUIA_AppMessage with a hook so that we get notified
        // as soon as the user drops a WB icon on the pagegroup object
        DoMethod(data->RG_PAGE, MUIM_Notify, MUIA_AppMessage, MUIV_EveryTime, MUIV_Notify_Application, 4, MUIM_CallHook, &AppMessageHook, MUIV_TriggerValue, obj);

        // set some notifications on the toolbar
        // in case it was generated
        if(data->TO_TOOLBAR != NULL)
        {
          // connect the buttons presses
          DoMethod(data->TO_TOOLBAR, MUIM_TheBar_Notify, TB_WRITE_EDITOR,    MUIA_Pressed, FALSE, obj, 1, MUIM_WriteWindow_LaunchEditor);
          DoMethod(data->TO_TOOLBAR, MUIM_TheBar_Notify, TB_WRITE_INSERT,    MUIA_Pressed, FALSE, obj, 2, MUIM_WriteWindow_EditorCmd, ED_INSERT);
          DoMethod(data->TO_TOOLBAR, MUIM_TheBar_Notify, TB_WRITE_CUT,       MUIA_Pressed, FALSE, data->TE_EDIT, 2, MUIM_TextEditor_ARexxCmd, "CUT");
          DoMethod(data->TO_TOOLBAR, MUIM_TheBar_Notify, TB_WRITE_COPY,      MUIA_Pressed, FALSE, data->TE_EDIT, 2, MUIM_TextEditor_ARexxCmd, "COPY");
          DoMethod(data->TO_TOOLBAR, MUIM_TheBar_Notify, TB_WRITE_PASTE,     MUIA_Pressed, FALSE, data->TE_EDIT, 2, MUIM_TextEditor_ARexxCmd, "PASTE");
          DoMethod(data->TO_TOOLBAR, MUIM_TheBar_Notify, TB_WRITE_UNDO,      MUIA_Pressed, FALSE, data->TE_EDIT, 2, MUIM_TextEditor_ARexxCmd, "UNDO");
          DoMethod(data->TO_TOOLBAR, MUIM_TheBar_Notify, TB_WRITE_BOLD,      MUIA_Selected, MUIV_EveryTime, obj, 3, MUIM_WriteWindow_SetSoftStyle, SSM_BOLD, ORIGIN_TOOLBAR);
          DoMethod(data->TO_TOOLBAR, MUIM_TheBar_Notify, TB_WRITE_ITALIC,    MUIA_Selected, MUIV_EveryTime, obj, 3, MUIM_WriteWindow_SetSoftStyle, SSM_ITALIC, ORIGIN_TOOLBAR);
          DoMethod(data->TO_TOOLBAR, MUIM_TheBar_Notify, TB_WRITE_UNDERLINE, MUIA_Selected, MUIV_EveryTime, obj, 3, MUIM_WriteWindow_SetSoftStyle, SSM_UNDERLINE, ORIGIN_TOOLBAR);
          DoMethod(data->TO_TOOLBAR, MUIM_TheBar_Notify, TB_WRITE_COLORED,   MUIA_Selected, MUIV_EveryTime, obj, 3, MUIM_WriteWindow_SetSoftStyle, SSM_COLOR, ORIGIN_TOOLBAR);
          DoMethod(data->TO_TOOLBAR, MUIM_TheBar_Notify, TB_WRITE_SEARCH,    MUIA_Pressed, FALSE, obj, 3, MUIM_WriteWindow_Search, MUIF_NONE);

          // connect attributes to button disables
          DoMethod(data->TE_EDIT, MUIM_Notify, MUIA_TextEditor_AreaMarked, MUIV_EveryTime, data->TO_TOOLBAR, 4, MUIM_TheBar_SetAttr, TB_WRITE_CUT, MUIA_TheBar_Attr_Disabled, MUIV_NotTriggerValue);
          DoMethod(data->TE_EDIT, MUIM_Notify, MUIA_TextEditor_AreaMarked, MUIV_EveryTime, data->TO_TOOLBAR, 4, MUIM_TheBar_SetAttr, TB_WRITE_COPY, MUIA_TheBar_Attr_Disabled, MUIV_NotTriggerValue);
          DoMethod(data->TE_EDIT, MUIM_Notify, MUIA_TextEditor_UndoAvailable, MUIV_EveryTime, data->TO_TOOLBAR, 4, MUIM_TheBar_SetAttr, TB_WRITE_UNDO, MUIA_TheBar_Attr_Disabled, MUIV_NotTriggerValue);

          // connect attributes to button selections
          // modifying the buttons' states must not cause any notifications!
          DoMethod(data->TE_EDIT, MUIM_Notify, MUIA_TextEditor_StyleBold,      MUIV_EveryTime, data->TO_TOOLBAR, 4, MUIM_TheBar_NoNotifySetAttr, TB_WRITE_BOLD,      MUIA_TheBar_Attr_Selected, MUIV_TriggerValue);
          DoMethod(data->TE_EDIT, MUIM_Notify, MUIA_TextEditor_StyleItalic,    MUIV_EveryTime, data->TO_TOOLBAR, 4, MUIM_TheBar_NoNotifySetAttr, TB_WRITE_ITALIC,    MUIA_TheBar_Attr_Selected, MUIV_TriggerValue);
          DoMethod(data->TE_EDIT, MUIM_Notify, MUIA_TextEditor_StyleUnderline, MUIV_EveryTime, data->TO_TOOLBAR, 4, MUIM_TheBar_NoNotifySetAttr, TB_WRITE_UNDERLINE, MUIA_TheBar_Attr_Selected, MUIV_TriggerValue);
          DoMethod(data->TE_EDIT, MUIM_Notify, MUIA_TextEditor_Pen,            0,              data->TO_TOOLBAR, 4, MUIM_TheBar_NoNotifySetAttr, TB_WRITE_COLORED,   MUIA_TheBar_Attr_Selected, FALSE);
          DoMethod(data->TE_EDIT, MUIM_Notify, MUIA_TextEditor_Pen,            6,              data->TO_TOOLBAR, 4, MUIM_TheBar_NoNotifySetAttr, TB_WRITE_COLORED,   MUIA_TheBar_Attr_Selected, FALSE);
          DoMethod(data->TE_EDIT, MUIM_Notify, MUIA_TextEditor_Pen,            7,              data->TO_TOOLBAR, 4, MUIM_TheBar_NoNotifySetAttr, TB_WRITE_COLORED,   MUIA_TheBar_Attr_Selected, TRUE);
          DoMethod(data->TE_EDIT, MUIM_Notify, MUIA_TextEditor_Pen,            8,              data->TO_TOOLBAR, 4, MUIM_TheBar_NoNotifySetAttr, TB_WRITE_COLORED,   MUIA_TheBar_Attr_Selected, FALSE);
          DoMethod(data->TE_EDIT, MUIM_Notify, MUIA_TextEditor_Pen,            9,              data->TO_TOOLBAR, 4, MUIM_TheBar_NoNotifySetAttr, TB_WRITE_COLORED,   MUIA_TheBar_Attr_Selected, FALSE);
          DoMethod(data->TE_EDIT, MUIM_Notify, MUIA_TextEditor_Pen,           10,              data->TO_TOOLBAR, 4, MUIM_TheBar_NoNotifySetAttr, TB_WRITE_COLORED,   MUIA_TheBar_Attr_Selected, FALSE);
          DoMethod(data->TE_EDIT, MUIM_Notify, MUIA_TextEditor_Pen,           11,              data->TO_TOOLBAR, 4, MUIM_TheBar_NoNotifySetAttr, TB_WRITE_COLORED,   MUIA_TheBar_Attr_Selected, FALSE);
          DoMethod(data->TE_EDIT, MUIM_Notify, MUIA_TextEditor_Pen,           12,              data->TO_TOOLBAR, 4, MUIM_TheBar_NoNotifySetAttr, TB_WRITE_COLORED,   MUIA_TheBar_Attr_Selected, FALSE);

          DoMethod(data->MI_BOLD,      MUIM_Notify, MUIA_Menuitem_Checked, MUIV_EveryTime, data->TO_TOOLBAR, 4, MUIM_TheBar_NoNotifySetAttr, TB_WRITE_BOLD,      MUIA_TheBar_Attr_Selected, MUIV_TriggerValue);
          DoMethod(data->MI_ITALIC,    MUIM_Notify, MUIA_Menuitem_Checked, MUIV_EveryTime, data->TO_TOOLBAR, 4, MUIM_TheBar_NoNotifySetAttr, TB_WRITE_ITALIC,    MUIA_TheBar_Attr_Selected, MUIV_TriggerValue);
          DoMethod(data->MI_UNDERLINE, MUIM_Notify, MUIA_Menuitem_Checked, MUIV_EveryTime, data->TO_TOOLBAR, 4, MUIM_TheBar_NoNotifySetAttr, TB_WRITE_UNDERLINE, MUIA_TheBar_Attr_Selected, MUIV_TriggerValue);
          DoMethod(data->MI_COLORED,   MUIM_Notify, MUIA_Menuitem_Checked, MUIV_EveryTime, data->TO_TOOLBAR, 4, MUIM_TheBar_NoNotifySetAttr, TB_WRITE_COLORED,   MUIA_TheBar_Attr_Selected, MUIV_TriggerValue);
        }

        if(data->TX_POSI != NULL)
        {
          DoMethod(data->TE_EDIT, MUIM_Notify, MUIA_TextEditor_CursorX, MUIV_EveryTime, obj, 1, MUIM_WriteWindow_UpdateCursorPos);
          DoMethod(data->TE_EDIT, MUIM_Notify, MUIA_TextEditor_CursorY, MUIV_EveryTime, obj, 1, MUIM_WriteWindow_UpdateCursorPos);
        }

        DoMethod(data->TE_EDIT, MUIM_Notify, MUIA_TextEditor_StyleBold,      MUIV_EveryTime, data->MI_BOLD,      3, MUIM_NoNotifySet, MUIA_Menuitem_Checked, MUIV_TriggerValue);
        DoMethod(data->TE_EDIT, MUIM_Notify, MUIA_TextEditor_StyleItalic,    MUIV_EveryTime, data->MI_ITALIC,    3, MUIM_NoNotifySet, MUIA_Menuitem_Checked, MUIV_TriggerValue);
        DoMethod(data->TE_EDIT, MUIM_Notify, MUIA_TextEditor_StyleUnderline, MUIV_EveryTime, data->MI_UNDERLINE, 3, MUIM_NoNotifySet, MUIA_Menuitem_Checked, MUIV_TriggerValue);
        DoMethod(data->TE_EDIT, MUIM_Notify, MUIA_TextEditor_Pen,            0,              data->MI_COLORED,   3, MUIM_NoNotifySet, MUIA_Menuitem_Checked, FALSE);
        DoMethod(data->TE_EDIT, MUIM_Notify, MUIA_TextEditor_Pen,            6,              data->MI_COLORED,   3, MUIM_NoNotifySet, MUIA_Menuitem_Checked, FALSE);
        DoMethod(data->TE_EDIT, MUIM_Notify, MUIA_TextEditor_Pen,            7,              data->MI_COLORED,   3, MUIM_NoNotifySet, MUIA_Menuitem_Checked, TRUE);
        DoMethod(data->TE_EDIT, MUIM_Notify, MUIA_TextEditor_Pen,            8,              data->MI_COLORED,   3, MUIM_NoNotifySet, MUIA_Menuitem_Checked, FALSE);
        DoMethod(data->TE_EDIT, MUIM_Notify, MUIA_TextEditor_Pen,            9,              data->MI_COLORED,   3, MUIM_NoNotifySet, MUIA_Menuitem_Checked, FALSE);
        DoMethod(data->TE_EDIT, MUIM_Notify, MUIA_TextEditor_Pen,           10,              data->MI_COLORED,   3, MUIM_NoNotifySet, MUIA_Menuitem_Checked, FALSE);
        DoMethod(data->TE_EDIT, MUIM_Notify, MUIA_TextEditor_Pen,           11,              data->MI_COLORED,   3, MUIM_NoNotifySet, MUIA_Menuitem_Checked, FALSE);
        DoMethod(data->TE_EDIT, MUIM_Notify, MUIA_TextEditor_Pen,           12,              data->MI_COLORED,   3, MUIM_NoNotifySet, MUIA_Menuitem_Checked, FALSE);

        DoMethod(data->MI_BOLD,      MUIM_Notify, MUIA_Menuitem_Checked, MUIV_EveryTime, obj, 3, MUIM_WriteWindow_SetSoftStyle, SSM_BOLD, ORIGIN_MENU);
        DoMethod(data->MI_ITALIC,    MUIM_Notify, MUIA_Menuitem_Checked, MUIV_EveryTime, obj, 3, MUIM_WriteWindow_SetSoftStyle, SSM_ITALIC, ORIGIN_MENU);
        DoMethod(data->MI_UNDERLINE, MUIM_Notify, MUIA_Menuitem_Checked, MUIV_EveryTime, obj, 3, MUIM_WriteWindow_SetSoftStyle, SSM_UNDERLINE, ORIGIN_MENU);
        DoMethod(data->MI_COLORED,   MUIM_Notify, MUIA_Menuitem_Checked, MUIV_EveryTime, obj, 3, MUIM_WriteWindow_SetSoftStyle, SSM_COLOR, ORIGIN_MENU);

        DoMethod(data->RG_PAGE,      MUIM_Notify, MUIA_Group_ActivePage,   0, MUIV_Notify_Window, 3, MUIM_Set, MUIA_Window_ActiveObject, data->TE_EDIT);
        DoMethod(data->RG_PAGE,      MUIM_Notify, MUIA_Group_ActivePage,   1, MUIV_Notify_Window, 3, MUIM_Set, MUIA_Window_ActiveObject, data->LV_ATTACH);
        DoMethod(data->ST_SUBJECT,   MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime, MUIV_Notify_Window, 3, MUIM_Set, MUIA_Window_ActiveObject, data->TE_EDIT);
        DoMethod(data->ST_SUBJECT,   MUIM_Notify, MUIA_String_Contents,    MUIV_EveryTime, obj, 1, MUIM_WriteWindow_UpdateWindowTitle);
        DoMethod(data->BT_ADD,       MUIM_Notify, MUIA_Pressed,            FALSE,          obj, 2, MUIM_WriteWindow_RequestAttachment, C->AttachDir);
        DoMethod(data->BT_ADDPACK,   MUIM_Notify, MUIA_Pressed,            FALSE,          obj, 1, MUIM_WriteWindow_AddArchive);
        DoMethod(data->BT_DEL,       MUIM_Notify, MUIA_Pressed,            FALSE,          obj, 1, MUIM_WriteWindow_DeleteAttachment);
        DoMethod(data->BT_DISPLAY,   MUIM_Notify, MUIA_Pressed,            FALSE,          obj, 1, MUIM_WriteWindow_DisplayAttachment);
        DoMethod(data->LV_ATTACH,    MUIM_Notify, MUIA_NList_DoubleClick,  MUIV_EveryTime, obj, 1, MUIM_WriteWindow_DisplayAttachment);
        DoMethod(data->LV_ATTACH,    MUIM_Notify, MUIA_NList_Active,       MUIV_EveryTime, obj, 1, MUIM_WriteWindow_GetAttachmentEntry);
        DoMethod(data->RA_ENCODING,  MUIM_Notify, MUIA_Radio_Active,       MUIV_EveryTime, obj, 1, MUIM_WriteWindow_PutAttachmentEntry);
        DoMethod(data->ST_CTYPE,     MUIM_Notify, MUIA_String_Contents,    MUIV_EveryTime, obj, 1, MUIM_WriteWindow_PutAttachmentEntry);
        DoMethod(data->ST_DESC,      MUIM_Notify, MUIA_String_Contents,    MUIV_EveryTime, obj, 1, MUIM_WriteWindow_PutAttachmentEntry);
        DoMethod(data->RA_SIGNATURE, MUIM_Notify, MUIA_Radio_Active,       MUIV_EveryTime, obj, 2, MUIM_WriteWindow_ChangeSignature, MUIV_TriggerValue);
        DoMethod(data->CH_DELSEND,   MUIM_Notify, MUIA_Selected,           MUIV_EveryTime, data->MI_DELSEND,        3, MUIM_Set,      MUIA_Menuitem_Checked, MUIV_TriggerValue);
        DoMethod(data->CH_MDN,       MUIM_Notify, MUIA_Selected,           MUIV_EveryTime, data->MI_MDN,            3, MUIM_Set,      MUIA_Menuitem_Checked, MUIV_TriggerValue);
        DoMethod(data->CH_ADDINFO,   MUIM_Notify, MUIA_Selected,           MUIV_EveryTime, data->MI_ADDINFO,        3, MUIM_Set,      MUIA_Menuitem_Checked, MUIV_TriggerValue);
        DoMethod(data->MI_AUTOSPELL, MUIM_Notify, MUIA_Menuitem_Checked,   MUIV_EveryTime, data->TE_EDIT,           3, MUIM_Set,      MUIA_TextEditor_TypeAndSpell, MUIV_TriggerValue);
        DoMethod(data->MI_AUTOWRAP,  MUIM_Notify, MUIA_Menuitem_Checked,   TRUE,           data->TE_EDIT,           3, MUIM_Set,      MUIA_TextEditor_WrapBorder, C->EdWrapCol);
        DoMethod(data->MI_AUTOWRAP,  MUIM_Notify, MUIA_Menuitem_Checked,   FALSE,          data->TE_EDIT,           3, MUIM_Set,      MUIA_TextEditor_WrapBorder, 0);
        DoMethod(data->MI_DELSEND,   MUIM_Notify, MUIA_Menuitem_Checked,   MUIV_EveryTime, data->CH_DELSEND,        3, MUIM_Set,      MUIA_Selected, MUIV_TriggerValue);
        DoMethod(data->MI_MDN,       MUIM_Notify, MUIA_Menuitem_Checked,   MUIV_EveryTime, data->CH_MDN,            3, MUIM_Set,      MUIA_Selected, MUIV_TriggerValue);
        DoMethod(data->MI_ADDINFO,   MUIM_Notify, MUIA_Menuitem_Checked,   MUIV_EveryTime, data->CH_ADDINFO,        3, MUIM_Set,      MUIA_Selected, MUIV_TriggerValue);
        DoMethod(data->RA_SECURITY,  MUIM_Notify, MUIA_Radio_Active,       4,              data->RA_SIGNATURE,      3, MUIM_Set,      MUIA_Radio_Active, 0);
        DoMethod(data->RA_SECURITY,  MUIM_Notify, MUIA_Radio_Active,       4,              data->CH_ADDINFO,        3, MUIM_Set,      MUIA_Selected, FALSE);
        DoMethod(data->MI_FFONT,     MUIM_Notify, MUIA_Menuitem_Checked,   MUIV_EveryTime, obj, 1, MUIM_WriteWindow_StyleOptionsChanged);
        DoMethod(data->MI_TCOLOR,    MUIM_Notify, MUIA_Menuitem_Checked,   MUIV_EveryTime, obj, 1, MUIM_WriteWindow_StyleOptionsChanged);
        DoMethod(data->MI_TSTYLE,    MUIM_Notify, MUIA_Menuitem_Checked,   MUIV_EveryTime, obj, 1, MUIM_WriteWindow_StyleOptionsChanged);

        // set the notifies for the importance cycle gadget
        DoMethod(data->CY_IMPORTANCE, MUIM_Notify, MUIA_Cycle_Active,      0,              menuStripObject,         4, MUIM_SetUData, WMEN_IMPORT0, MUIA_Menuitem_Checked, TRUE);
        DoMethod(obj,                 MUIM_Notify, MUIA_Window_MenuAction, WMEN_IMPORT0,   data->CY_IMPORTANCE,     3, MUIM_Set,      MUIA_Cycle_Active, 0);
        DoMethod(data->CY_IMPORTANCE, MUIM_Notify, MUIA_Cycle_Active,      1,              menuStripObject,         4, MUIM_SetUData, WMEN_IMPORT1, MUIA_Menuitem_Checked, TRUE);
        DoMethod(obj,                 MUIM_Notify, MUIA_Window_MenuAction, WMEN_IMPORT1,   data->CY_IMPORTANCE,     3, MUIM_Set,      MUIA_Cycle_Active, 1);
        DoMethod(data->CY_IMPORTANCE, MUIM_Notify, MUIA_Cycle_Active,      2,              menuStripObject,         4, MUIM_SetUData, WMEN_IMPORT2, MUIA_Menuitem_Checked, TRUE);
        DoMethod(obj,                 MUIM_Notify, MUIA_Window_MenuAction, WMEN_IMPORT2,   data->CY_IMPORTANCE,     3, MUIM_Set,      MUIA_Cycle_Active, 2);

        // set the notifies for the signature radio gadget
        DoMethod(data->RA_SIGNATURE,  MUIM_Notify, MUIA_Radio_Active,      0,              menuStripObject,         4, MUIM_SetUData, WMEN_SIGN0, MUIA_Menuitem_Checked, TRUE);
        DoMethod(obj,                 MUIM_Notify, MUIA_Window_MenuAction, WMEN_SIGN0,     data->RA_SIGNATURE,      3, MUIM_Set,      MUIA_Radio_Active, 0);
        DoMethod(data->RA_SIGNATURE,  MUIM_Notify, MUIA_Radio_Active,      1,              menuStripObject,         4, MUIM_SetUData, WMEN_SIGN1, MUIA_Menuitem_Checked, TRUE);
        DoMethod(obj,                 MUIM_Notify, MUIA_Window_MenuAction, WMEN_SIGN1,     data->RA_SIGNATURE,      3, MUIM_Set,      MUIA_Radio_Active, 1);
        DoMethod(data->RA_SIGNATURE,  MUIM_Notify, MUIA_Radio_Active,      2,              menuStripObject,         4, MUIM_SetUData, WMEN_SIGN2, MUIA_Menuitem_Checked, TRUE);
        DoMethod(obj,                 MUIM_Notify, MUIA_Window_MenuAction, WMEN_SIGN2,     data->RA_SIGNATURE,      3, MUIM_Set,      MUIA_Radio_Active, 2);
        DoMethod(data->RA_SIGNATURE,  MUIM_Notify, MUIA_Radio_Active,      3,              menuStripObject,         4, MUIM_SetUData, WMEN_SIGN3, MUIA_Menuitem_Checked, TRUE);
        DoMethod(obj,                 MUIM_Notify, MUIA_Window_MenuAction, WMEN_SIGN3,     data->RA_SIGNATURE,      3, MUIM_Set,      MUIA_Radio_Active, 3);

        // set the notifies for the security radio gadget
        DoMethod(data->RA_SECURITY,   MUIM_Notify, MUIA_Radio_Active,      0,              menuStripObject,         4, MUIM_SetUData, WMEN_SECUR0, MUIA_Menuitem_Checked, TRUE);
        DoMethod(obj,                 MUIM_Notify, MUIA_Window_MenuAction, WMEN_SECUR0,    data->RA_SECURITY,       3, MUIM_Set,      MUIA_Radio_Active, 0);
        DoMethod(data->RA_SECURITY,   MUIM_Notify, MUIA_Radio_Active,      1,              menuStripObject,         4, MUIM_SetUData, WMEN_SECUR1, MUIA_Menuitem_Checked, TRUE);
        DoMethod(obj,                 MUIM_Notify, MUIA_Window_MenuAction, WMEN_SECUR1,    data->RA_SECURITY,       3, MUIM_Set,      MUIA_Radio_Active, 1);
        DoMethod(data->RA_SECURITY,   MUIM_Notify, MUIA_Radio_Active,      2,              menuStripObject,         4, MUIM_SetUData, WMEN_SECUR2, MUIA_Menuitem_Checked, TRUE);
        DoMethod(obj,                 MUIM_Notify, MUIA_Window_MenuAction, WMEN_SECUR2,    data->RA_SECURITY,       3, MUIM_Set,      MUIA_Radio_Active, 2);
        DoMethod(data->RA_SECURITY,   MUIM_Notify, MUIA_Radio_Active,      3,              menuStripObject,         4, MUIM_SetUData, WMEN_SECUR3, MUIA_Menuitem_Checked, TRUE);
        DoMethod(obj,                 MUIM_Notify, MUIA_Window_MenuAction, WMEN_SECUR3,    data->RA_SECURITY,       3, MUIM_Set,      MUIA_Radio_Active, 3);
        DoMethod(data->RA_SECURITY,   MUIM_Notify, MUIA_Radio_Active,      4,              menuStripObject,         4, MUIM_SetUData, WMEN_SECUR4, MUIA_Menuitem_Checked, TRUE);
        DoMethod(obj,                 MUIM_Notify, MUIA_Window_MenuAction, WMEN_SECUR4,    data->RA_SECURITY,       3, MUIM_Set,      MUIA_Radio_Active, 4);
        DoMethod(data->RA_SECURITY,   MUIM_Notify, MUIA_Radio_Active,      5,              menuStripObject,         4, MUIM_SetUData, WMEN_SECUR5, MUIA_Menuitem_Checked, TRUE);
        DoMethod(obj,                 MUIM_Notify, MUIA_Window_MenuAction, WMEN_SECUR5,    data->RA_SECURITY,       3, MUIM_Set,      MUIA_Radio_Active, 5);

        // set some default values for this newly created
        // write window
        setstring(data->ST_FROM, BuildAddress(address, sizeof(address), C->EmailAddress, C->RealName));
        setstring(data->ST_REPLYTO, C->ReplyTo);
        setstring(data->ST_EXTHEADER, C->ExtraHeaders);
        setcheckmark(data->CH_DELSEND, !C->SaveSent);
        setcheckmark(data->CH_ADDINFO, C->AddMyInfo);
        setcheckmark(data->CH_MDN, C->RequestMDN);
      }

      // notify stuff shared by all mail modes

      SetHelp(data->ST_TO,        MSG_HELP_WR_ST_TO);
      SetHelp(data->BT_QUEUE,     MSG_HELP_WR_BT_QUEUE);
      SetHelp(data->BT_HOLD,      MSG_HELP_WR_BT_HOLD);
      SetHelp(data->BT_SEND,      MSG_HELP_WR_BT_SEND);
      SetHelp(data->BT_CANCEL,    MSG_HELP_WR_BT_CANCEL);
      SetHelp(data->PO_CHARSET,   MSG_HELP_WR_PO_CHARSET);

      // set main window button notifies
      DoMethod(data->BT_HOLD,       MUIM_Notify, MUIA_Pressed, FALSE, obj, 2, MUIM_WriteWindow_ComposeMail, WRITE_HOLD);
      DoMethod(data->BT_QUEUE,      MUIM_Notify, MUIA_Pressed, FALSE, obj, 2, MUIM_WriteWindow_ComposeMail, WRITE_QUEUE);
      DoMethod(data->BT_SEND,       MUIM_Notify, MUIA_Pressed, FALSE, obj, 2, MUIM_WriteWindow_ComposeMail, WRITE_SEND);
      DoMethod(data->BT_CANCEL,     MUIM_Notify, MUIA_Pressed, FALSE, obj, 1, MUIM_WriteWindow_CancelAction);

      // connect the closerequest attribute to the cancelaction method so that
      // users might get informed of an eventually data loss
      DoMethod(obj, MUIM_Notify, MUIA_Window_CloseRequest, TRUE, obj, 1, MUIM_WriteWindow_CancelAction);

      // prepare the temporary filename of that new write window
      snprintf(filename, sizeof(filename), "YAMw%08x-%d.tmp", (unsigned int)FindTask(NULL), data->windowNumber);
      AddPath(data->wmData->filename, C->TempDir, filename, sizeof(data->wmData->filename));

      // set the global charset as the default one
      data->wmData->charset = G->writeCharset;

      // Finally set up the notifications for external changes to the file being edited
      // if this is not a bounce window. We let the UserData point to this object to be
      // able to invoke a method from the main loop. However, we only do this for write
      // windows which are not opened in bounce mode
      if(data->wmData->mode != NMM_BOUNCE)
      {
        #if defined(__amigaos4__)
        data->wmData->notifyRequest = AllocDosObjectTags(DOS_NOTIFYREQUEST, ADO_NotifyName,     data->wmData->filename,
                                                                            ADO_NotifyUserData, (ULONG)obj,
                                                                            ADO_NotifyMethod,   NRF_SEND_MESSAGE,
                                                                            ADO_NotifyPort,     G->writeWinNotifyPort,
                                                                            TAG_DONE);
        #else
        if((data->wmData->notifyRequest = AllocVecPooled(G->SharedMemPool, sizeof(*data->wmData->notifyRequest))) != NULL)
        {
          data->wmData->notifyRequest->nr_Name = data->wmData->filename;
          data->wmData->notifyRequest->nr_UserData = (ULONG)obj;
          data->wmData->notifyRequest->nr_Flags = NRF_SEND_MESSAGE;
          data->wmData->notifyRequest->nr_stuff.nr_Msg.nr_Port = G->writeWinNotifyPort;
        }
        #endif
      }
      else
        data->wmData->notifyRequest = NULL;

      // there is no active notification yet
      data->wmData->fileNotifyActive = FALSE;

      // place our data in the node and add it to the writeMailDataList
      AddTail((struct List *)&(G->writeMailDataList), (struct Node *)data->wmData);

      if(data->wmData->mode != NMM_BOUNCE)
      {
        // finally set up the notifications for external changes to the file being edited if this is not a bounce window
        if((data->notifyPort = AllocSysObjectTags(ASOT_PORT, TAG_DONE)) != NULL)
        {
          #if defined(__amigaos4__)
          data->notifyRequest = AllocDosObjectTags(DOS_NOTIFYREQUEST, ADO_NotifyName, data->wmData->filename,
                                                                      ADO_NotifyMethod, NRF_SEND_MESSAGE,
                                                                      ADO_NotifyPort, data->notifyPort,
                                                                      TAG_DONE);
          #else
          if((data->notifyRequest = AllocVecPooled(G->SharedMemPool, sizeof(*data->notifyRequest))) != NULL)
          {
            data->notifyRequest->nr_Name = data->wmData->filename;
            data->notifyRequest->nr_Flags = NRF_SEND_MESSAGE;
            data->notifyRequest->nr_stuff.nr_Msg.nr_Port = data->notifyPort;
          }
          #endif

          if(data->notifyRequest != NULL)
          {
            StartNotify(data->notifyRequest);
          }
        }
      }

      // we created a new write window, lets
      // go and start the PREWRITE macro
      MA_StartMacro(MACRO_PREWRITE, itoa(data->windowNumber));
    }
  }

  // free the temporary mem we allocated before
  free(tmpData);

  RETURN((ULONG)obj);
  return (ULONG)obj;
}

///
/// OVERLOAD(OM_DISPOSE)
OVERLOAD(OM_DISPOSE)
{
  GETDATA;
  ULONG result;

  ENTER();

  if(data->wmData->mode != NMM_BOUNCE)
  {
    char fileName[SIZE_PATHFILE];
    int i;

    // cleanup the attachment list
    for(i=0; ;i++)
    {
      struct Attach *att;

      DoMethod(data->LV_ATTACH, MUIM_NList_GetEntry, i, &att);
      if(att == NULL)
        break;

      if(att->IsTemp == TRUE)
        DeleteFile(att->FilePath);
    }

    if(data->notifyRequest != NULL)
    {
      EndNotify(data->notifyRequest);

      #if defined(__amigaos4__)
      FreeDosObject(DOS_NOTIFYREQUEST, data->notifyRequest);
      #else
      FreeVecPooled(G->SharedMemPool, data->notifyRequest);
      #endif
      data->notifyRequest = NULL;
    }

    if(data->notifyPort != NULL)
    {
      FreeSysObject(ASOT_PORT, data->notifyPort);
      data->notifyPort = NULL;
    }

    // delete a possible autosave file
    DeleteFile(WR_AutoSaveFile(data->windowNumber, fileName, sizeof(fileName)));
  }

  // check the reference window ptr of the addressbook
  if(G->AB->winNumber == data->windowNumber)
    G->AB->winNumber = -1;

  // signal the super class to dispose as well
  result = DoSuperMethodA(cl, obj, msg);

  RETURN(result);
  return result;
}

///
/// OVERLOAD(OM_GET)
OVERLOAD(OM_GET)
{
  GETDATA;
  IPTR *store = ((struct opGet *)msg)->opg_Storage;

  switch(((struct opGet *)msg)->opg_AttrID)
  {
    case ATTR(WriteMailData): *store = (ULONG)data->wmData; return TRUE;
    case ATTR(Num):           *store = data->windowNumber; return TRUE;
    case ATTR(To):            *store = xget(data->ST_TO, MUIA_String_Contents) ; return TRUE;
    case ATTR(Quiet):         *store = data->wmData->quietMode; return TRUE;
    case ATTR(NotifyPort):    *store = (ULONG)data->notifyPort; return TRUE;
  }

  return DoSuperMethodA(cl, obj, msg);
}
///
/// OVERLOAD(OM_SET)
OVERLOAD(OM_SET)
{
  GETDATA;
  struct TagItem *tags = inittags(msg), *tag;

  while((tag = NextTagItem((APTR)&tags)) != NULL)
  {
    switch(tag->ti_Tag)
    {
      case ATTR(ActiveObject):
      {
        Object *actObj = NULL;

        switch((enum ActiveObject)tag->ti_Data)
        {
          case MUIV_WriteWindow_ActiveObject_To:
            actObj = data->ST_TO;
          break;

          case MUIV_WriteWindow_ActiveObject_TextEditor:
            actObj = data->TE_EDIT;
          break;

          case MUIV_WriteWindow_ActiveObject_Subject:
            actObj = data->ST_SUBJECT;
          break;
        }

        if(actObj != NULL)
          set(obj, MUIA_Window_ActiveObject, actObj);
      }
      break;

      case ATTR(To):
      {
        setstring(data->ST_TO, tag->ti_Data);

        // make the superMethod call ignore those tags
        tag->ti_Tag = TAG_IGNORE;
      }
      break;

      case ATTR(Cc):
      {
        setstring(data->ST_CC, tag->ti_Data);

        // make the superMethod call ignore those tags
        tag->ti_Tag = TAG_IGNORE;
      }
      break;

      case ATTR(BCC):
      {
        setstring(data->ST_BCC, tag->ti_Data);

        // make the superMethod call ignore those tags
        tag->ti_Tag = TAG_IGNORE;
      }
      break;

      case ATTR(ReplyTo):
      {
        setstring(data->ST_REPLYTO, tag->ti_Data);

        // make the superMethod call ignore those tags
        tag->ti_Tag = TAG_IGNORE;
      }
      break;

      case ATTR(From):
      {
        setstring(data->ST_FROM, tag->ti_Data);

        // make the superMethod call ignore those tags
        tag->ti_Tag = TAG_IGNORE;
      }
      break;

      case ATTR(ExtHeaders):
      {
        setstring(data->ST_EXTHEADER, tag->ti_Data);

        // make the superMethod call ignore those tags
        tag->ti_Tag = TAG_IGNORE;
      }
      break;

      case ATTR(Subject):
      {
        setstring(data->ST_SUBJECT, tag->ti_Data);

        // make the superMethod call ignore those tags
        tag->ti_Tag = TAG_IGNORE;
      }
      break;

      case ATTR(AttachDescription):
      {
        setstring(data->ST_DESC, tag->ti_Data);

        // make the superMethod call ignore those tags
        tag->ti_Tag = TAG_IGNORE;
      }
      break;

      case ATTR(AttachEncoding):
      {
        setmutex(data->RA_ENCODING, tag->ti_Data);

        // make the superMethod call ignore those tags
        tag->ti_Tag = TAG_IGNORE;
      }
      break;

      case ATTR(AttachContentType):
      {
        setstring(data->ST_CTYPE, tag->ti_Data);

        // make the superMethod call ignore those tags
        tag->ti_Tag = TAG_IGNORE;
      }
      break;

      case ATTR(SendDisabled):
      {
        set(data->BT_SEND, MUIA_Disabled, tag->ti_Data);

        // make the superMethod call ignore those tags
        tag->ti_Tag = TAG_IGNORE;
      }
      break;

      case ATTR(DelSend):
      {
        setcheckmark(data->CH_DELSEND, tag->ti_Data);

        // make the superMethod call ignore those tags
        tag->ti_Tag = TAG_IGNORE;
      }
      break;

      case ATTR(MDN):
      {
        setcheckmark(data->CH_MDN, tag->ti_Data);

        // make the superMethod call ignore those tags
        tag->ti_Tag = TAG_IGNORE;
      }
      break;

      case ATTR(AddInfo):
      {
        setcheckmark(data->CH_ADDINFO, tag->ti_Data);

        // make the superMethod call ignore those tags
        tag->ti_Tag = TAG_IGNORE;
      }
      break;

      case ATTR(Importance):
      {
        setcycle(data->CY_IMPORTANCE, tag->ti_Data);

        // make the superMethod call ignore those tags
        tag->ti_Tag = TAG_IGNORE;
      }
      break;

      case ATTR(Signature):
      {
        setmutex(data->RA_SIGNATURE, tag->ti_Data);

        // make the superMethod call ignore those tags
        tag->ti_Tag = TAG_IGNORE;
      }
      break;

      case ATTR(Security):
      {
        setmutex(data->RA_SECURITY, tag->ti_Data);

        // make the superMethod call ignore those tags
        tag->ti_Tag = TAG_IGNORE;
      }
      break;

      case ATTR(MailBody):
      {
        set(data->TE_EDIT, MUIA_TextEditor_Contents, tag->ti_Data);

        // make the superMethod call ignore those tags
        tag->ti_Tag = TAG_IGNORE;
      }
      break;
    }
  }

  return DoSuperMethodA(cl, obj, msg);
}
///

/* Private Methods */
/// DECLARE(EditActionPerformed)
//  User pressed an item of the edit submenu (cut/copy/paste, etc)
DECLARE(EditActionPerformed) // enum EditAction action
{
  GETDATA;
  Object *actObj;

  ENTER();

  // now we check what active object we currently got
  // so that we can send the menu action to the correct
  // gadget
  if((actObj = (Object *)xget(obj, MUIA_Window_ActiveObject)) != NULL)
  {
    // check which action we got
    switch(msg->action)
    {
      case EA_CUT:
      {
        if(actObj == data->TE_EDIT)
          DoMethod(actObj, MUIM_TextEditor_ARexxCmd, "CUT");
        else if(actObj == data->ST_TO || actObj == data->ST_SUBJECT ||
                actObj == data->ST_CC || actObj == data->ST_BCC ||
                actObj == data->ST_FROM || actObj == data->ST_REPLYTO ||
                actObj == data->ST_EXTHEADER || actObj == data->ST_DESC ||
                actObj == data->ST_CTYPE)
        {
          DoMethod(actObj, MUIM_BetterString_DoAction, MUIV_BetterString_DoAction_Cut);
        }
      }
      break;

      case EA_COPY:
      {
        if(actObj == data->TE_EDIT)
          DoMethod(actObj, MUIM_TextEditor_ARexxCmd, "COPY");
        else if(actObj == data->ST_TO || actObj == data->ST_SUBJECT ||
                actObj == data->ST_CC || actObj == data->ST_BCC ||
                actObj == data->ST_FROM || actObj == data->ST_REPLYTO ||
                actObj == data->ST_EXTHEADER || actObj == data->ST_DESC ||
                actObj == data->ST_CTYPE)
        {
          DoMethod(actObj, MUIM_BetterString_DoAction, MUIV_BetterString_DoAction_Copy);
        }
        else if(actObj == data->LV_ATTACH)
        {
          DoMethod(actObj, MUIM_NList_CopyToClip, MUIV_NList_CopyToClip_Active, 0, NULL, NULL);
        }
      }
      break;

      case EA_PASTE:
      {
        if(actObj == data->TE_EDIT)
          DoMethod(actObj, MUIM_TextEditor_ARexxCmd, "PASTE");
        else if(actObj == data->ST_TO || actObj == data->ST_SUBJECT ||
                actObj == data->ST_CC || actObj == data->ST_BCC ||
                actObj == data->ST_FROM || actObj == data->ST_REPLYTO ||
                actObj == data->ST_EXTHEADER || actObj == data->ST_DESC ||
                actObj == data->ST_CTYPE)
        {
          DoMethod(actObj, MUIM_BetterString_DoAction, MUIV_BetterString_DoAction_Paste);
        }
      }
      break;

      case EA_DELETE:
      {
        if(actObj == data->TE_EDIT)
          DoMethod(actObj, MUIM_TextEditor_ARexxCmd, "DELETE");
        else if(actObj == data->ST_TO || actObj == data->ST_SUBJECT ||
                actObj == data->ST_CC || actObj == data->ST_BCC ||
                actObj == data->ST_FROM || actObj == data->ST_REPLYTO ||
                actObj == data->ST_EXTHEADER || actObj == data->ST_DESC ||
                actObj == data->ST_CTYPE)
        {
          DoMethod(actObj, MUIM_BetterString_DoAction, MUIV_BetterString_DoAction_Delete);
        }
      }
      break;

      case EA_UNDO:
      {
        if(actObj == data->TE_EDIT)
          DoMethod(actObj, MUIM_TextEditor_ARexxCmd, "UNDO");
        else if(actObj == data->ST_TO || actObj == data->ST_SUBJECT ||
                actObj == data->ST_CC || actObj == data->ST_BCC ||
                actObj == data->ST_FROM || actObj == data->ST_REPLYTO ||
                actObj == data->ST_EXTHEADER || actObj == data->ST_DESC ||
                actObj == data->ST_CTYPE)
        {
          DoMethod(actObj, MUIM_BetterString_DoAction, MUIV_BetterString_DoAction_Undo);
        }
      }
      break;

      case EA_REDO:
      {
        if(actObj == data->TE_EDIT)
          DoMethod(actObj, MUIM_TextEditor_ARexxCmd, "REDO");
        else if(actObj == data->ST_TO || actObj == data->ST_SUBJECT ||
                actObj == data->ST_CC || actObj == data->ST_BCC ||
                actObj == data->ST_FROM || actObj == data->ST_REPLYTO ||
                actObj == data->ST_EXTHEADER || actObj == data->ST_DESC ||
                actObj == data->ST_CTYPE)
        {
          DoMethod(actObj, MUIM_BetterString_DoAction, MUIV_BetterString_DoAction_Redo);
        }
      }
      break;

      case EA_SELECTALL:
      {
        if(actObj == data->TE_EDIT)
          DoMethod(actObj, MUIM_TextEditor_ARexxCmd, "SELECTALL");
        else if(actObj == data->ST_TO || actObj == data->ST_SUBJECT ||
                actObj == data->ST_CC || actObj == data->ST_BCC ||
                actObj == data->ST_FROM || actObj == data->ST_REPLYTO ||
                actObj == data->ST_EXTHEADER || actObj == data->ST_DESC ||
                actObj == data->ST_CTYPE)
        {
          DoMethod(actObj, MUIM_BetterString_DoAction, MUIV_BetterString_DoAction_SelectAll);
        }
      }
      break;

      case EA_SELECTNONE:
      {
        if(actObj == data->TE_EDIT)
          DoMethod(actObj, MUIM_TextEditor_ARexxCmd, "SELECTNONE");
        else if(actObj == data->ST_TO || actObj == data->ST_SUBJECT ||
                actObj == data->ST_CC || actObj == data->ST_BCC ||
                actObj == data->ST_FROM || actObj == data->ST_REPLYTO ||
                actObj == data->ST_EXTHEADER || actObj == data->ST_DESC ||
                actObj == data->ST_CTYPE)
        {
          DoMethod(actObj, MUIM_BetterString_DoAction, MUIV_BetterString_DoAction_SelectNone);
        }
      }
      break;
    }
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(RequestAttachment)
// Requests to enter a filename and add one or more files to the attachment list
DECLARE(RequestAttachment) // STRPTR drawer
{
  struct FileReqCache *frc;

  ENTER();

  if((frc = ReqFile(ASL_ATTACH, obj, tr(MSG_WR_AddFile), REQF_MULTISELECT, msg->drawer, "")))
  {
    char filename[SIZE_PATHFILE];

    if(frc->numArgs == 0)
    {
      D(DBF_GUI, "chosen file: [%s] from drawer: [%s]", frc->file, frc->drawer);

      AddPath(filename, frc->drawer, frc->file, sizeof(filename));

      DoMethod(obj, MUIM_WriteWindow_AddAttachment, filename, NULL, FALSE);
    }
    else
    {
      int i;

      for(i=0; i < frc->numArgs; i++)
      {
        D(DBF_GUI, "chosen file: [%s] from drawer: [%s]", frc->argList[i], frc->drawer);

        AddPath(filename, frc->drawer, frc->argList[i], sizeof(filename));

        DoMethod(obj, MUIM_WriteWindow_AddAttachment, filename, NULL, FALSE);
      }
    }
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(AddClipboard)
// Adds contents of clipboard as attachment ***/
DECLARE(AddClipboard)
{
  struct TempFile *tf;

  ENTER();

  if((tf = OpenTempFile("w")) != NULL)
  {
    BOOL dumped = FALSE;

    if(DumpClipboard(tf->FP))
    {
      fclose(tf->FP);
      tf->FP = NULL;

      DoMethod(obj, MUIM_WriteWindow_AddAttachment, tf->Filename, "clipboard.text", TRUE);
      free(tf);

      // don't delete this file by CloseTempFile()
      dumped = TRUE;
    }

    if(dumped == FALSE)
      CloseTempFile(tf);
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(AddPGPKey)
// Adds ASCII version of user's public PGP key as attachment ***/
DECLARE(AddPGPKey)
{
  GETDATA;
  char *myid = *C->MyPGPID ? C->MyPGPID : C->EmailAddress;
  char options[SIZE_LARGE];
  const char  *fname = "T:PubKey.asc";

  ENTER();

  snprintf(options, sizeof(options), (G->PGPVersion == 5) ? "-x %s -o %s +force +batchmode=1" : "-kxa %s %s +f +bat", myid, fname);

  if(!PGPCommand((G->PGPVersion == 5) ? "pgpk" : "pgp", options, 0))
  {
    LONG size;

    if(ObtainFileInfo(fname, FI_SIZE, &size) == TRUE && size > 0)
    {
      DoMethod(obj, MUIM_WriteWindow_AddAttachment, fname, NULL, TRUE);
      setstring(data->ST_CTYPE, "application/pgp-keys");
    }
    else
      ER_NewError(tr(MSG_ER_ErrorAppendKey), myid);
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(UpdateCusorPos)
// Shows/Updates cursor coordinates in write window
DECLARE(UpdateCursorPos)
{
  GETDATA;
  ENTER();

  snprintf(data->cursorPos, sizeof(data->cursorPos), "%03ld\n%03ld", xget(data->TE_EDIT, MUIA_TextEditor_CursorY)+1,
                                                                     xget(data->TE_EDIT, MUIA_TextEditor_CursorX)+1);
  set(data->TX_POSI, MUIA_Text_Contents, data->cursorPos);

  RETURN(0);
  return 0;
}

///
/// DECLARE(UpdateWindowTitle)
//  As soon as the subject string gadget is going to be
//  changed, this hook will be called.
DECLARE(UpdateWindowTitle)
{
  GETDATA;
  char *subject;
  size_t titleLen;

  ENTER();

  subject = (char *)xget(data->ST_SUBJECT, MUIA_String_Contents);
  titleLen = snprintf(data->windowTitle, sizeof(data->windowTitle), "[%d] %s: ", data->windowNumber+1, tr(MSG_WR_WriteWT));

  if(strlen(subject)+titleLen > sizeof(data->windowTitle)-1)
  {
    if(titleLen < sizeof(data->windowTitle)-4)
    {
      strlcat(data->windowTitle, subject, sizeof(data->windowTitle)-titleLen-4);
      strlcat(data->windowTitle, "...", sizeof(data->windowTitle)); // signals that the string was cut.
    }
    else
      strlcat(&data->windowTitle[sizeof(data->windowTitle)-5], "...", 4);
  }
  else
    strlcat(data->windowTitle, subject, sizeof(data->windowTitle));

  set(obj, MUIA_Window_Title, data->windowTitle);

  RETURN(0);
  return 0;
}

///
/// DECLARE(AddArchive)
// Creates an archive of one or more files and adds it to the attachment list
DECLARE(AddArchive)
{
  BOOL result = FALSE;
  struct FileReqCache *frc;

  ENTER();

  // request files from the user via asl.library
  if((frc = ReqFile(ASL_ATTACH, obj, tr(MSG_WR_AddFile), REQF_MULTISELECT, C->AttachDir, "")) &&
      frc->numArgs > 0)
  {
    char *p;
    char arcname[SIZE_FILE];

    // get the basename of the first selected file and prepare it to the user
    // as a suggestion for an archive name
    strlcpy(arcname, FilePart(frc->argList[0]), sizeof(arcname));
    if((p = strrchr(arcname, '.')) != NULL)
      *p = '\0';

    // now ask the user for the choosen archive name
    if(StringRequest(arcname, SIZE_FILE, tr(MSG_WR_CreateArc), tr(MSG_WR_CreateArcReq), tr(MSG_Okay), NULL, tr(MSG_Cancel), FALSE, obj))
    {
      char *command;
      char arcpath[SIZE_PATHFILE];
      struct TempFile *tf = NULL;

      // create the destination archive name
      AddPath(arcpath, C->TempDir, arcname, sizeof(arcpath));

      // now we generate the temporary file containing the file names
      // which we are going to put in the archive.
      if(strstr(C->PackerCommand, "%l") != NULL && (tf = OpenTempFile("w")) != NULL)
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
      if((command = AllocStrBuf(SIZE_DEFAULT)) != NULL)
      {
        char *src;
        BPTR filedir;
        BOOL mustCloseQuote = FALSE;

        for(src = C->PackerCommand; *src != '\0'; src++)
        {
          if(*src == '%')
          {
            src++;
            switch(*src)
            {
              case '%':
              {
                command = StrBufCat(command, "%");
              }
              break;

              case 'a':
              {
                D(DBF_UTIL, "insert archive name '%s'", arcpath);
                if(strchr(arcpath, ' ') != NULL)
                {
                  // surround the file name by quotes if it contains spaces
                  command = StrBufCat(command, "\"");
                  command = StrBufCat(command, arcpath);
                  // remember to add the closing quotes
                  mustCloseQuote = TRUE;
                }
                else
                  command = StrBufCat(command, arcpath);
              }
              break;

              case 'l':
              {
                D(DBF_UTIL, "insert filename '%s'", tf->Filename);
                if(strchr(tf->Filename, ' ') != NULL)
                {
                  // surround the file name by quotes if it contains spaces
                  command = StrBufCat(command, "\"");
                  command = StrBufCat(command, tf->Filename);
                  // remember to add the closing quotes
                  mustCloseQuote = TRUE;
                }
                else
                  command = StrBufCat(command, tf->Filename);
              }
              break;

              case 'f':
              {
                int i;

                for(i=0; i < frc->numArgs; i++)
                {
                  char filename[SIZE_PATHFILE];

                  snprintf(filename, sizeof(filename), "\"%s\" ", frc->argList[i]);
                  command = StrBufCat(command, filename);
                }
                break;
              }
              break;
            }
          }
          else if(*src == ' ')
          {
            // if we are to insert a space character and there are quotes
            // to be closed we do it now and forget about the quotes afterwards.
            if(mustCloseQuote == TRUE)
            {
              command = StrBufCat(command, "\" ");
              mustCloseQuote = FALSE;
            }
            else
            {
              // no quotes to be closed, just add the space character
              command = StrBufCat(command, " ");
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

        // if there are still quotes to be closed do it now
        if(mustCloseQuote == TRUE)
          command = StrBufCat(command, "\"");

        // now we make the request drawer the current one temporarly.
        if((filedir = Lock(frc->drawer, ACCESS_READ)) != 0)
        {
          BPTR olddir = CurrentDir(filedir);

          // now we do the archive generation right here.
          if(LaunchCommand(command, FALSE, OUT_NIL) == RETURN_OK)
            result = TRUE;

          // make the old directory the new current one again
          CurrentDir(olddir);
          UnLock(filedir);
        }

        // free our private resources
        FreeStrBuf(command);
        CloseTempFile(tf);

        // if everything worked out fine we go
        // and find out the real final attachment name
        if(result == TRUE)
        {
          APTR oldwin;
          LONG size;
          char filename[SIZE_PATHFILE];

          // don't let DOS bother us with requesters while we check some files
          oldwin = SetProcWindow((APTR)-1);

          strlcpy(filename, arcpath, sizeof(filename));
          if(ObtainFileInfo(filename, FI_SIZE, &size) == FALSE)
          {
            snprintf(filename, sizeof(filename), "%s.lha", arcpath);
            if(ObtainFileInfo(filename, FI_SIZE, &size) == FALSE)
            {
              snprintf(filename, sizeof(filename), "%s.lzx", arcpath);
              if(ObtainFileInfo(filename, FI_SIZE, &size) == FALSE)
                snprintf(filename, sizeof(filename), "%s.zip", arcpath);
            }
          }

          // allow requesters from DOS again
          SetProcWindow(oldwin);

          DoMethod(obj, MUIM_WriteWindow_AddAttachment, filename, NULL, TRUE);
        }
        else
          ER_NewError(tr(MSG_ER_PACKERROR));
      }
    }
  }

  RETURN(result);
  return result;
}

///
/// DECLARE(DeleteAttachment)
// Deletes a file from the attachment list and the belonging temporary file
DECLARE(DeleteAttachment)
{
  GETDATA;
  struct Attach *attach = NULL;

  ENTER();

  // first we get the active entry
  DoMethod(data->LV_ATTACH, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &attach);
  if(attach != NULL)
  {
    // then we remove the active entry from the NList
    DoMethod(data->LV_ATTACH, MUIM_NList_Remove, MUIV_NList_Remove_Active);

    // delete the temporary file if exists
    if(attach->IsTemp == TRUE)
      DeleteFile(attach->FilePath);
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(DisplayAttachment)
// Displays an attached file using a MIME viewer
DECLARE(DisplayAttachment)
{
  GETDATA;
  struct Attach *attach = NULL;

  ENTER();

  DoMethod(data->LV_ATTACH, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &attach);
  if(attach != NULL)
  {
    if(FileExists(attach->FilePath))
      RE_DisplayMIME(attach->FilePath, attach->ContentType);
    else
      ER_NewError(tr(MSG_ER_INVALIDATTFILE), attach->FilePath);
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(GetAttachmentEntry)
// Fills form with data from selected list entry
DECLARE(GetAttachmentEntry)
{
  GETDATA;
  struct Attach *attach = NULL;

  ENTER();

  DoMethod(data->LV_ATTACH, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &attach);
  DoMethod(G->App, MUIM_MultiSet, MUIA_Disabled, attach ? FALSE : TRUE, data->RA_ENCODING,
                                                                        data->ST_CTYPE,
                                                                        data->ST_DESC,
                                                                        data->BT_DEL,
                                                                        data->BT_DISPLAY, NULL);

  if(attach != NULL)
  {
    nnset(data->RA_ENCODING, MUIA_Radio_Active, attach->IsMIME ? 0 : 1);
    nnset(data->ST_CTYPE, MUIA_String_Contents, attach->ContentType);
    nnset(data->ST_DESC, MUIA_String_Contents, attach->Description);
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(PutAttachmentEntry)
// Fills form data into selected list entry
DECLARE(PutAttachmentEntry)
{
  GETDATA;
  struct Attach *attach = NULL;

  ENTER();

  DoMethod(data->LV_ATTACH, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &attach);
  if(attach != NULL)
  {
    int ismime = xget(data->RA_ENCODING, MUIA_Radio_Active);

    attach->IsMIME = (ismime == 0);
    GetMUIString(attach->ContentType, data->ST_CTYPE, sizeof(attach->ContentType));
    GetMUIString(attach->Description, data->ST_DESC, sizeof(attach->Description));
    DoMethod(data->LV_ATTACH, MUIM_NList_Redraw, MUIV_NList_Redraw_Active);
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(ChangeSignature)
// Changes the current signature
DECLARE(ChangeSignature) // LONG signature
{
  GETDATA;
  struct TempFile *tfin;

  ENTER();

  if((tfin = OpenTempFile(NULL)) != NULL)
  {
    FILE *in;
    Object *editor = data->TE_EDIT;

    DoMethod(editor, MUIM_MailTextEdit_SaveToFile, tfin->Filename);
    if((in = fopen(tfin->Filename, "r")) != NULL)
    {
      struct TempFile *tfout;

      setvbuf(in, NULL, _IOFBF, SIZE_FILEBUF);

      // open a new temporary file for writing the new text
      if((tfout = OpenTempFile("w")) != NULL)
      {
        char *buf = NULL;
        size_t buflen = 0;
        ssize_t curlen;
        ULONG flags;

        while((curlen = getline(&buf, &buflen, in)) > 0)
        {
          if(strcmp(buf, "-- \n") == 0)
            break;

          fwrite(buf, curlen, 1, tfout->FP);
        }

        if(msg->signature > 0)
          WriteSignature(tfout->FP, msg->signature-1);

        // now our out file is finished, so we can
        // put everything in our text editor.
        fclose(tfout->FP);
        tfout->FP = NULL;

        // free the buffer
        free(buf);

        // put everything in the editor
        flags = MUIF_NONE;
        if(data->useTextStyles == TRUE)
          SET_FLAG(flags, MUIF_MailTextEdit_LoadFromFile_UseStyles);
        if(data->useTextColors == TRUE)
         SET_FLAG(flags, MUIF_MailTextEdit_LoadFromFile_UseColors);
        DoMethod(editor, MUIM_MailTextEdit_LoadFromFile, tfout->Filename, xget(editor, MUIA_TextEditor_HasChanged), flags);

        // make sure the temp file is deleted
        CloseTempFile(tfout);
      }

      fclose(in);
    }

    CloseTempFile(tfin);
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(SetSoftStyle)
DECLARE(SetSoftStyle) // enum SoftStyleMode ssm, ULONG origin
{
  GETDATA;
  char *txt;

  ENTER();

  // retrieve the whole text lines where the cursor/block is currently active.
  if((txt = (char *)DoMethod(data->TE_EDIT, MUIM_TextEditor_ExportBlock, MUIF_TextEditor_ExportBlock_FullLines)))
  {
    int txtlen = strlen(txt);

    if(txtlen > 0)
    {
      ULONG x1=0;
      ULONG y1=0;
      ULONG x2=0;
      ULONG y2=0;
      ULONG firstChar=0;
      ULONG lastChar=0;
      LONG nx1;
      LONG nx2;
      ULONG ny1;
      char marker[2] = " ";
      BOOL enableStyle = FALSE;
      ULONG toolbarStore = 0;
      LONG addedChars = 0;
      LONG lastAddedChars = 0;

      D(DBF_GUI, "txt '%s', origin %ld", txt, msg->origin);

      // define the marker for the
      // selected soft-style and check if
      // we should enable or disable the style
      switch(msg->ssm)
      {
        case SSM_NORMAL:
          // nothing;
        break;

        case SSM_BOLD:
        {
          marker[0] = '*';
          if(msg->origin == ORIGIN_MENU)
            enableStyle = (xget(data->MI_BOLD, MUIA_Menuitem_Checked) == TRUE);
          else if(msg->origin == ORIGIN_TOOLBAR)
          {
            DoMethod(data->TO_TOOLBAR, MUIM_TheBar_GetAttr, TB_WRITE_BOLD, MUIA_TheBar_Attr_Selected, &toolbarStore);
            enableStyle = (toolbarStore != 0);
          }
        }
        break;

        case SSM_ITALIC:
        {
          marker[0] = '/';
          if(msg->origin == ORIGIN_MENU)
            enableStyle = (xget(data->MI_ITALIC, MUIA_Menuitem_Checked) == TRUE);
          else if(msg->origin == ORIGIN_TOOLBAR)
          {
            DoMethod(data->TO_TOOLBAR, MUIM_TheBar_GetAttr, TB_WRITE_ITALIC, MUIA_TheBar_Attr_Selected, &toolbarStore);
            enableStyle = (toolbarStore != 0);
          }
        }
        break;

        case SSM_UNDERLINE:
        {
          marker[0] = '_';
          if(msg->origin == ORIGIN_MENU)
            enableStyle = (xget(data->MI_UNDERLINE, MUIA_Menuitem_Checked) == TRUE);
          else if(msg->origin == ORIGIN_TOOLBAR)
          {
            DoMethod(data->TO_TOOLBAR, MUIM_TheBar_GetAttr, TB_WRITE_UNDERLINE, MUIA_TheBar_Attr_Selected, &toolbarStore);
            enableStyle = (toolbarStore != 0);
          }
        }
        break;

        case SSM_COLOR:
        {
          marker[0] = '#';
          if(msg->origin == ORIGIN_MENU)
            enableStyle = (xget(data->MI_COLORED, MUIA_Menuitem_Checked) == TRUE);
          else if(msg->origin == ORIGIN_TOOLBAR)
          {
            DoMethod(data->TO_TOOLBAR, MUIM_TheBar_GetAttr, TB_WRITE_COLORED, MUIA_TheBar_Attr_Selected, &toolbarStore);
            enableStyle = (toolbarStore != 0);
          }
        }
        break;
      }

      D(DBF_GUI, "marker '%s', enable %ld, toolbarStore %ld", marker, enableStyle, toolbarStore);

      // check/get the status of the marked area
      if(DoMethod(data->TE_EDIT, MUIM_TextEditor_BlockInfo, &x1, &y1, &x2, &y2) == FALSE)
      {
        x1 = xget(data->TE_EDIT, MUIA_TextEditor_CursorX);
        y1 = xget(data->TE_EDIT, MUIA_TextEditor_CursorY);
        x2 = 0;
        y2 = y1;
      }

      // set the n* values we use to parse
      // through the retrieved txt
      nx1=x1;
      nx2=x1;
      ny1=y1;

      // we first walk back from the starting (nx1) position searching for the a
      // space or the start of line
      while(nx1 > 0 && !isspace(txt[nx1]))
        nx1--;

      if(nx1 > 0 && (ULONG)nx1 != x1)
        nx1++;

      // now we search forward from nx2 and find the end of the word
      // by searching for the next space
      while(nx2 < txtlen && !isspace(txt[nx2]))
        nx2++;

      // lets set x1 to the actual nx1 we just identified
      firstChar = nx1;
      lastChar = nx2;

      // make the texteditor be quiet during our
      // operations
      set(data->TE_EDIT, MUIA_TextEditor_Quiet, TRUE);

      // prepare to walk through our selected area
      do
      {
        // if we found a string between nx1 and nx2 we can now
        // enable or disable the style for that particular area
        if(nx2-nx1 > 0)
        {
          char *ntxt = NULL;

          if(enableStyle == TRUE)
          {
            // check if there is already some soft-style active
            // and if yes, we go and strip them
            if(msg->ssm == SSM_NORMAL ||
               (txt[nx1] != marker[0] || txt[nx2-1] != marker[0]))
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
                {
                  strlcat(ntxt, &txt[nx1], nx2-nx1+2);
                  addedChars += 2;
                }

                strlcat(ntxt, marker, nx2-nx1+2+1);
              }
            }
          }
          else
          {
            // check if there is already some soft-style active
            // and if yes, we go and strip them
            if(msg->ssm != SSM_NORMAL &&
               (txt[nx1] == marker[0] && txt[nx2-1] == marker[0]))
            {
              if((ntxt = calloc(1, nx2-2-nx1+1+1)))
                strlcpy(ntxt, &txt[nx1+1], nx2-2-nx1+1);

              addedChars -= 2;
            }
          }

          if(ntxt)
          {
            D(DBF_GUI, "ntxt: '%s' : %ld %ld", ntxt, enableStyle, addedChars);

            // now we mark the area we found to be the one we want to
            // set bold/italic whatever
            DoMethod(data->TE_EDIT, MUIM_TextEditor_MarkText, nx1+lastAddedChars, ny1, nx2+lastAddedChars, ny1);

            // now we replace the marked area with the text we extract but adding two
            // bold signs at the back and setting the area bold
            DoMethod(data->TE_EDIT, MUIM_TextEditor_Replace, ntxt, MUIF_NONE);

            // set the soft-style accordingly.
            if(enableStyle == TRUE)
            {
              DoMethod(data->TE_EDIT, MUIM_TextEditor_MarkText, nx1+lastAddedChars, ny1, nx1+lastAddedChars+strlen(ntxt), ny1);

              // make sure the text in question is really in the style we want
              switch(msg->ssm)
              {
                case SSM_NORMAL:
                {
                  xset(data->TE_EDIT, MUIA_TextEditor_StyleBold,      FALSE,
                                      MUIA_TextEditor_StyleItalic,    FALSE,
                                      MUIA_TextEditor_StyleUnderline, FALSE,
                                      MUIA_TextEditor_Pen,            0);
                }
                break;

                case SSM_BOLD:
                {
                  xset(data->TE_EDIT, MUIA_TextEditor_StyleBold,      data->useTextStyles,
                                      MUIA_TextEditor_StyleItalic,    FALSE,
                                      MUIA_TextEditor_StyleUnderline, FALSE,
                                      MUIA_TextEditor_Pen,            0);
                }
                break;

                case SSM_ITALIC:
                {
                  xset(data->TE_EDIT, MUIA_TextEditor_StyleBold,      FALSE,
                                      MUIA_TextEditor_StyleItalic,    data->useTextStyles,
                                      MUIA_TextEditor_StyleUnderline, FALSE,
                                      MUIA_TextEditor_Pen,            0);
                }
                break;

                case SSM_UNDERLINE:
                {
                  xset(data->TE_EDIT, MUIA_TextEditor_StyleBold,      FALSE,
                                      MUIA_TextEditor_StyleItalic,    FALSE,
                                      MUIA_TextEditor_StyleUnderline, data->useTextStyles,
                                      MUIA_TextEditor_Pen,            0);
                }
                break;

                case SSM_COLOR:
                {
                  xset(data->TE_EDIT, MUIA_TextEditor_StyleBold,      FALSE,
                                      MUIA_TextEditor_StyleItalic,    FALSE,
                                      MUIA_TextEditor_StyleUnderline, FALSE,
                                      MUIA_TextEditor_Pen,            data->useTextColors == TRUE ? 7 : 0);
                }
                break;
              }
            }

            free(ntxt);
          }

          lastChar = nx2+addedChars;
          lastAddedChars = addedChars;
        }
        else
          break;

        // now we check wheter there is more to process or not.
        if(x2 > (ULONG)nx2+1)
        {
          nx1 = ++nx2; // set nx1 to end of last string

          // now we search forward from nx1 and find the start of the
          // next word
          while(nx1 < txtlen && isspace(txt[nx1]))
            nx1++;

          // search for the end of the found word
          nx2 = nx1;
          while(nx2 < txtlen && !isspace(txt[nx2]))
            nx2++;
        }
        else
          nx2 = nx1;
      }
      while(TRUE);

      // mark all text we just processed
      DoMethod(data->TE_EDIT, MUIM_TextEditor_MarkText, firstChar, ny1, lastChar, ny1);

      // turn the texteditor back on
      set(data->TE_EDIT, MUIA_TextEditor_Quiet, FALSE);
    }

    FreeVec(txt);
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(StyleOptionsChanged)
DECLARE(StyleOptionsChanged)
{
  GETDATA;
  BOOL updateText = FALSE;
  BOOL tmp;

  ENTER();

  // check useTextcolors diff
  if((tmp = xget(data->MI_TCOLOR, MUIA_Menuitem_Checked)) != data->useTextColors)
  {
    data->useTextColors = tmp;
    updateText = TRUE;
  }

  // check useTextstyles diff
  if((tmp = xget(data->MI_TSTYLE, MUIA_Menuitem_Checked)) != data->useTextStyles)
  {
    data->useTextStyles = tmp;
    updateText = TRUE;
  }

  // check useFixedFont diff
  if((tmp = xget(data->MI_FFONT, MUIA_Menuitem_Checked)) != data->useFixedFont)
  {
    data->useFixedFont = tmp;

    // update the font settings of TE_EDIT
    set(data->TE_EDIT, MUIA_TextEditor_FixedFont, data->useFixedFont);
  }

  // issue an update of the readMailGroup's components
  if(updateText == TRUE)
  {
    char *orgtext;

    // get the original text from TE_EDIT
    orgtext = (char *)DoMethod(data->TE_EDIT, MUIM_TextEditor_ExportText);

    if(orgtext != NULL)
    {
      char *parsedText;

      // parse the text and do some highlighting and stuff
      if((parsedText = ParseEmailText(orgtext, FALSE, data->useTextStyles, data->useTextColors)) != NULL)
      {
        // set the new text and preserve the changed status
        set(data->TE_EDIT, MUIA_TextEditor_Contents, parsedText);

        free(parsedText);
      }

      FreeVec(orgtext); // use FreeVec() because TextEditor.mcc uses AllocVec()
    }
    else
      E(DBF_GUI, "couldn't export text from TE_EDIT");
  }

  RETURN(0);
  return 0;
}
///

/* Public Methods */
/// DECLARE(SaveTextAs)
// Saves contents of internal editor to a file
DECLARE(SaveTextAs)
{
  GETDATA;
  struct FileReqCache *frc;

  ENTER();

  if(xget(obj, MUIA_Window_Open) == TRUE)
    set(data->RG_PAGE, MUIA_Group_ActivePage, 0);

  if((frc = ReqFile(ASL_ATTACH, obj, tr(MSG_WR_SaveTextAs), REQF_SAVEMODE, C->AttachDir, "")))
  {
    char filename[SIZE_PATHFILE];

    AddPath(filename, frc->drawer, frc->file, sizeof(filename));

    if(FileExists(filename) == FALSE ||
       MUI_Request(G->App, obj, 0, tr(MSG_MA_ConfirmReq), tr(MSG_YesNoReq), tr(MSG_FILE_OVERWRITE), frc->file) != 0)
    {
      DoMethod(data->TE_EDIT, MUIM_MailTextEdit_SaveToFile, data->wmData->filename);

      if(CopyFile(filename, NULL, data->wmData->filename, NULL) == FALSE)
        ER_NewError(tr(MSG_ER_CantCreateFile), filename);
    }
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(AddAttachment)
// Adds a file to the attachment list, gets its size and type
DECLARE(AddAttachment) // const char *filename, const char *name, ULONG istemp
{
  GETDATA;
  BOOL result = FALSE;

  ENTER();

  if(msg->filename != NULL)
  {
    static struct Attach attach;
    const char *ctype;
    #if defined(__amigaos4__)
    struct ExamineData *ed;
    #else
    BPTR lock;
    #endif

    memset(&attach, 0, sizeof(struct Attach));

    // we try to get the filesize and comment and copy
    // it into our attchement structure.
    #if defined(__amigaos4__)
    if((ed = ExamineObjectTags(EX_StringName, msg->filename, TAG_DONE)) != NULL)
    {
      attach.Size = ed->FileSize;
      strlcpy(attach.Description, ed->Comment, sizeof(attach.Description));

      // check the dir type and protection
      if(EXD_IS_FILE(ed) && isFlagClear(ed->Protection, EXDF_READ))
        result = TRUE;

      FreeDosObject(DOS_EXAMINEDATA, ed);
    }
    #else
    if((lock = Lock((STRPTR)msg->filename, ACCESS_READ)))
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
    #endif

    // now we try to find out the content-type of the
    // attachment by using the IdentifyFile() function which will
    // do certain checks to guess the content-type out of the file name
    // and content.
    if(result == TRUE && (ctype = IdentifyFile(msg->filename)) != NULL && ctype[0] != '\0')
    {
      int encoding = xget(data->RA_ENCODING, MUIA_Radio_Active);

      attach.IsMIME = (encoding == 0);
      attach.IsTemp = msg->istemp;
      strlcpy(attach.FilePath, msg->filename, sizeof(attach.FilePath));
      strlcpy(attach.Name, msg->name ? msg->name : (char *)FilePart(msg->filename), sizeof(attach.Name));
      strlcpy(attach.ContentType, ctype, sizeof(attach.ContentType));
      nnset(data->ST_CTYPE, MUIA_String_Contents, attach.ContentType);
      nnset(data->ST_DESC, MUIA_String_Contents, attach.Description);

      // put the attachment into our attachment MUI list and set
      // it active.
      DoMethod(data->LV_ATTACH, MUIM_NList_InsertSingle, &attach, MUIV_NList_Insert_Bottom);
      set(data->LV_ATTACH, MUIA_NList_Active, MUIV_NList_Active_Bottom);
    }
    else
    {
      ER_NewError(tr(MSG_ER_ADDFILEERROR), msg->filename);

      result = FALSE;
    }
  }

  RETURN(result);
  return result;
}
///
/// DECLARE(InsertAttachment)
// Insert an attachment directly with a preconfigured struct Attach
DECLARE(InsertAttachment) // struct Attach *attach
{
  GETDATA;
  ULONG result;
  ENTER();

  result = DoMethod(data->LV_ATTACH, MUIM_NList_InsertSingle, msg->attach, MUIV_NList_Insert_Bottom);

  RETURN(result);
  return result;
}

///
/// DECLARE(AddSignature)
//  Adds a signature to the end of the file
DECLARE(AddSignature) // int signat
{
  GETDATA;
  int signat = msg->signat;

  ENTER();

  if(signat == -1)
    signat = C->UseSignature ? 1 : 0;

  if(signat)
  {
    FILE *fh_mail;
    BOOL addline = FALSE;

    if((fh_mail = fopen(data->wmData->filename, "r")) != NULL)
    {
      fseek(fh_mail, -1, SEEK_END);
      addline = fgetc(fh_mail) != '\n';
      fclose(fh_mail);
    }

    if((fh_mail = fopen(data->wmData->filename, "a")) != NULL)
    {
      setvbuf(fh_mail, NULL, _IOFBF, SIZE_FILEBUF);

      if(addline)
        fputc('\n', fh_mail);

      WriteSignature(fh_mail, signat-1);
      fclose(fh_mail);
    }
  }

  // lets set the signature radiobutton
  // accordingly to the set signature
  nnset(data->RA_SIGNATURE, MUIA_Radio_Active, signat);

  RETURN(0);
  return 0;
}

///
/// DECLARE(AddRecipient)
//  Adds a recipient to one of the recipient types (To/Cc/BCc)
DECLARE(AddRecipient) // enum RcptType type, char *recipient
{
  GETDATA;
  ENTER();

  switch(msg->type)
  {
    case MUIV_WriteWindow_RcptType_To:
      DoMethod(data->ST_TO, MUIM_Recipientstring_AddRecipient, msg->recipient);
    break;

    case MUIV_WriteWindow_RcptType_Cc:
      DoMethod(data->ST_CC, MUIM_Recipientstring_AddRecipient, msg->recipient);
    break;

    case MUIV_WriteWindow_RcptType_BCC:
      DoMethod(data->ST_BCC, MUIM_Recipientstring_AddRecipient, msg->recipient);
    break;

    case MUIV_WriteWindow_RcptType_ReplyTo:
      DoMethod(data->ST_REPLYTO, MUIM_Recipientstring_AddRecipient, msg->recipient);
    break;

    case MUIV_WriteWindow_RcptType_From:
      DoMethod(data->ST_FROM, MUIM_Recipientstring_AddRecipient, msg->recipient);
    break;
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(InsertAddresses)
//  Appends an array of addresses to a string gadget
DECLARE(InsertAddresses) // enum RcptType type, char **addr, ULONG add
{
  GETDATA;
  Object *str = NULL;

  ENTER();

  ENTER();

  switch(msg->type)
  {
    case MUIV_WriteWindow_RcptType_To:
      str = data->ST_TO;
    break;

    case MUIV_WriteWindow_RcptType_Cc:
      str = data->ST_CC;
    break;

    case MUIV_WriteWindow_RcptType_BCC:
      str = data->ST_BCC;
    break;

    case MUIV_WriteWindow_RcptType_ReplyTo:
      str = data->ST_REPLYTO;
    break;

    case MUIV_WriteWindow_RcptType_From:
      str = data->ST_FROM;
    break;
  }

  if(str != NULL)
  {
    if(msg->add == FALSE)
      setstring(str, "");

    do
    {
      DoMethod(str, MUIM_Recipientstring_AddRecipient, *(msg->addr));
      ++msg->addr;
    }
    while(*(msg->addr) != NULL);
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(LaunchEditor)
// Launches external editor with message text
DECLARE(LaunchEditor)
{
  GETDATA;
  ENTER();

  if(C->Editor[0] != '\0')
  {
    struct WriteMailData *wmData = data->wmData;
    char buffer[SIZE_COMMAND+SIZE_PATHFILE];

    // stop any pending file notification.
    if(wmData->fileNotifyActive == TRUE)
    {
      EndNotify(wmData->notifyRequest);
      wmData->fileNotifyActive = FALSE;
    }

    // Workaround for a MUI bug
    if(xget(obj, MUIA_Window_Open) == TRUE)
      set(data->RG_PAGE, MUIA_Group_ActivePage, 0);

    DoMethod(data->TE_EDIT, MUIM_MailTextEdit_SaveToFile, data->wmData->filename);
    // remember the modification date of the file
    if(ObtainFileInfo(data->wmData->filename, FI_DATE, &data->wmData->lastFileChangeTime) == FALSE)
    {
      // use the current time in case ObtainFileInfo() failed
      DateStamp(&data->wmData->lastFileChangeTime);
    }

    snprintf(buffer, sizeof(buffer), "%s \"%s\"", C->Editor, GetRealPath(wmData->filename));
    LaunchCommand(buffer, TRUE, OUT_NIL);

    // (re)start the file notification on the temporary write window
    // content file
    // start the notification
    if(StartNotify(wmData->notifyRequest) != 0)
    {
      D(DBF_UTIL, "started notification request for file: '%s' of write window %ld", wmData->filename, data->windowNumber);
      wmData->fileNotifyActive = TRUE;
    }
    else
      W(DBF_UTIL, "file notification [%s] of write window %ld failed!", wmData->filename, data->windowNumber);
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(EditorCmd)
// Inserts file or clipboard into editor
DECLARE(EditorCmd) // enum TransformMode cmd
{
  GETDATA;
  char *text = NULL;
  char *quoteChar;
  char filename[SIZE_PATHFILE];
  struct TempFile *tf;

  ENTER();

  quoteChar = (msg->cmd == ED_INSALTQUOT || msg->cmd == ED_PASALTQUOT) ? C->AltQuoteChar : C->QuoteChar;

  switch(msg->cmd)
  {
    case ED_INSERT:
    case ED_INSQUOT:
    case ED_INSALTQUOT:
    case ED_INSROT13:
    case ED_OPEN:
    {
      struct FileReqCache *frc;

      if((frc = ReqFile(ASL_ATTACH, obj, tr(MSG_WR_InsertFile), REQF_NONE, C->AttachDir, "")))
      {
        AddPath(filename, frc->drawer, frc->file, sizeof(filename));
        text = TransformText(filename, msg->cmd, quoteChar);
      }
      else
      {
        RETURN(0);
        return 0;
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
      if((frc = ReqFile(ASL_ATTACH, obj, tr(MSG_WR_InsertFile), REQF_NONE, C->AttachDir, "")))
      {
        AddPath(filename, frc->drawer, frc->file, sizeof(filename));

        // open a temporary file
        if((tf = OpenTempFile("w")))
        {
          FILE *in;

          // open both files and start the uuencoding
          if((in = fopen(filename, "r")))
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
              text = TransformText(tf->Filename, msg->cmd, filename);
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
        text = TransformText(tf->Filename, msg->cmd, quoteChar);
        CloseTempFile(tf);
      }
      else
      {
        RETURN(0);
        return 0;
      }
    }
    break;
  }

  if(text != NULL)
  {
    if(msg->cmd == ED_OPEN)
      DoMethod(data->TE_EDIT, MUIM_TextEditor_ClearText);

    DoMethod(data->TE_EDIT, MUIM_TextEditor_InsertText, text);

    // free our allocated text
    free(text);
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(ArexxCommand)
// forward a string command to the TE.mcc object
DECLARE(ArexxCommand) // char *command
{
  GETDATA;
  ULONG result;
  ENTER();

  result = DoMethod(data->TE_EDIT, MUIM_TextEditor_ARexxCmd, msg->command);

  RETURN(result);
  return result;
}

///
/// DECLARE(Search)
// Opens a search subwindow
DECLARE(Search) // ULONG flags
{
  GETDATA;
  ENTER();

  if(data->WI_SEARCH == NULL)
  {
    if((data->WI_SEARCH = SearchwindowObject, End))
    {
      // perform the search operation
      DoMethod(data->WI_SEARCH, MUIM_Searchwindow_Open, data->TE_EDIT);
    }
  }
  else
  {
    if(hasSearchAgainFlag(msg->flags))
      DoMethod(data->WI_SEARCH, MUIM_Searchwindow_Next);
    else
      DoMethod(data->WI_SEARCH, MUIM_Searchwindow_Open, data->TE_EDIT);
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(InsertText)
// Insert a certain text into the editor
DECLARE(InsertText) // char *text
{
  GETDATA;
  ENTER();

  DoMethod(data->TE_EDIT, MUIM_TextEditor_InsertText, msg->text);

  RETURN(0);
  return 0;
}

///
/// DECLARE(ReloadText)
// Reload the message text from the current temporary file
DECLARE(ReloadText) // ULONG changed
{
  GETDATA;
  BOOL result;
  ULONG flags;

  ENTER();

  flags = MUIF_NONE;
  if(msg->changed != FALSE)
    SET_FLAG(flags, MUIF_MailTextEdit_LoadFromFile_SetChanged);
  if(data->useTextStyles == TRUE)
    SET_FLAG(flags, MUIF_MailTextEdit_LoadFromFile_UseStyles);
  if(data->useTextColors == TRUE)
    SET_FLAG(flags, MUIF_MailTextEdit_LoadFromFile_UseColors);
  result = DoMethod(data->TE_EDIT, MUIM_MailTextEdit_LoadFromFile, data->wmData->filename, flags);

  RETURN(result);
  return (ULONG)result;
}

///
/// DECLARE(LoadText)
// Load the message text from a specified file
DECLARE(LoadText) // char *filename, ULONG changed
{
  GETDATA;
  BOOL result;
  ULONG flags;

  ENTER();

  flags = MUIF_NONE;
  if(msg->changed != FALSE)
    SET_FLAG(flags, MUIF_MailTextEdit_LoadFromFile_SetChanged);
  if(data->useTextStyles == TRUE)
    SET_FLAG(flags, MUIF_MailTextEdit_LoadFromFile_UseStyles);
  if(data->useTextColors == TRUE)
    SET_FLAG(flags, MUIF_MailTextEdit_LoadFromFile_UseColors);
  result = DoMethod(data->TE_EDIT, MUIM_MailTextEdit_LoadFromFile, msg->filename, flags);

  RETURN(result);
  return (ULONG)result;
}

///
/// DECLARE(SetupFromOldMail)
// When editing a message, sets write window options to old values ***/
DECLARE(SetupFromOldMail) // struct ReadMailData *rmData
{
  GETDATA;
  struct Part *part;

  ENTER();

  // we start to iterate right from the first part *after* PART_RAW
  // and check which one really is really an attachment or which
  // one is the LetterPart
  for(part = msg->rmData->firstPart->Next; part; part = part->Next)
  {
    if(part->Nr == msg->rmData->letterPartNum)
    {
      // find the selected charset and default to the global one if it
      // could not be found
      if((data->wmData->charset = CodesetsFind(part->CParCSet,
                                                      CSA_CodesetList,       G->codesetsList,
                                                      CSA_FallbackToDefault, FALSE,
                                                      TAG_DONE)) == NULL)
      {
        // fallback to global writeCharset
        data->wmData->charset = G->writeCharset;
      }

      nnset(data->PO_CHARSET, MUIA_Text_Contents, strippedCharsetName(data->wmData->charset));
    }

    if(part->Nr != msg->rmData->letterPartNum &&
       stricmp(part->ContentType, "application/pgp-signature"))
    {
      struct Attach attach;

      memset(&attach, 0, sizeof(struct Attach));

      BusyText(tr(MSG_BusyDecSaving), "");

      RE_DecodePart(part);

      attach.Size = part->Size;
      attach.IsMIME = part->EncodingCode != ENC_UUE;
      attach.IsTemp = TRUE;

      if(part->Name)
        strlcpy(attach.Name, part->Name, sizeof(attach.Name));

      strlcpy(attach.FilePath, part->Filename, sizeof(attach.FilePath));
      *part->Filename = '\0';
      strlcpy(attach.ContentType, part->ContentType, sizeof(attach.ContentType));
      strlcpy(attach.Description, part->Description, sizeof(attach.Description));

      DoMethod(data->LV_ATTACH, MUIM_NList_InsertSingle, &attach, MUIV_NList_Insert_Bottom);

      BusyEnd();
    }
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(GetAttachment)
DECLARE(GetAttachment) // int num, struct Attach *att
{
  GETDATA;
  ULONG result;
  ENTER();

  result = DoMethod(data->LV_ATTACH, MUIM_NList_GetEntry, msg->num, msg->att);

  RETURN(result);
  return result;
}

///
/// DECLARE(IsEditorActive)
DECLARE(IsEditorActive)
{
  GETDATA;
  ULONG result = 0;
  ENTER();

  result = ((Object *)xget(obj, MUIA_Window_ActiveObject) == data->TE_EDIT) ? 1 : 0;

  RETURN(result);
  return result;
}

///
/// DECLARE(ComposeMail)
//  Validates write window options and generates a new message
DECLARE(ComposeMail) // enum WriteMode mode
{
  GETDATA;
  char newMailFile[SIZE_PATHFILE];
  struct Compose comp;
  struct Mail *newMail = NULL;
  char *addr;
  int numAttachments = 0;
  struct Folder *outfolder = FO_GetFolderByType(FT_OUTGOING, NULL);
  BOOL winOpen = xget(obj, MUIA_Window_Open);
  struct WriteMailData *wmData = data->wmData;
  enum WriteMode mode = msg->mode;

  ENTER();

  // Workaround for a MUI bug
  if(winOpen == TRUE)
    set(data->RG_PAGE, MUIA_Group_ActivePage, xget(data->RG_PAGE, MUIA_Group_ActivePage));

  // clear some variables we fill up later on
  memset(&comp, 0, sizeof(struct Compose));

  // first we check all input values and fill up
  // the struct Compose variable

  // get the contents of the TO: String gadget and check if it is valid
  addr = (char *)DoMethod(data->ST_TO, MUIM_Recipientstring_Resolve, MUIF_Recipientstring_Resolve_NoValid);
  if(addr == NULL)
  {
    ER_NewError(tr(MSG_ER_AliasNotFound), (STRPTR)xget(data->ST_TO, MUIA_String_Contents));

    if(winOpen == TRUE)
      set(data->RG_PAGE, MUIA_Group_ActivePage, 0);

    set(obj, MUIA_Window_ActiveObject, data->ST_TO);

    RETURN(0);
    return 0;
  }
  else if(addr[0] == '\0' && wmData->quietMode == FALSE)
  {
    // set the TO Field active and go back
    if(winOpen == TRUE)
      set(data->RG_PAGE, MUIA_Group_ActivePage, 0);

    set(obj, MUIA_Window_ActiveObject, data->ST_TO);

    if(MUI_Request(G->App, obj, 0, NULL, tr(MSG_WR_NoRcptReqGad), tr(MSG_WR_ErrorNoRcpt)) != 0)
      mode = WRITE_HOLD;
    else
    {
      RETURN(0);
      return 0;
    }
  }
  else
    comp.MailTo = addr; // To: address

  // get the content of the Subject: String gadget and check if it is empty or not.
  comp.Subject = (char *)xget(data->ST_SUBJECT, MUIA_String_Contents);
  if(wmData->mode != NMM_BOUNCE && wmData->quietMode == FALSE && C->WarnSubject == TRUE &&
     (comp.Subject == NULL || comp.Subject[0] == '\0'))
  {
    if(winOpen == TRUE)
      set(data->RG_PAGE, MUIA_Group_ActivePage, 0);

    set(obj, MUIA_Window_ActiveObject, data->ST_SUBJECT);

    if(MUI_Request(G->App, obj, 0, NULL, tr(MSG_WR_OKAYCANCELREQ), tr(MSG_WR_NOSUBJECTREQ)) == 0)
    {
      RETURN(0);
      return 0;
    }
  }

  comp.Mode = wmData->mode;
  comp.refMail = wmData->refMail;
  comp.OldSecurity = wmData->oldSecurity;

  if(wmData->mode != NMM_BOUNCE)
  {
    // now we check the From gadget and raise an error if is invalid
    addr = (char *)DoMethod(data->ST_FROM, MUIM_Recipientstring_Resolve, MUIF_Recipientstring_Resolve_NoValid);
    if(addr == NULL)
    {
      ER_NewError(tr(MSG_ER_AliasNotFound), (STRPTR)xget(data->ST_FROM, MUIA_String_Contents));

      if(winOpen == TRUE)
        set(data->RG_PAGE, MUIA_Group_ActivePage, 2);

      set(obj, MUIA_Window_ActiveObject, data->ST_FROM);

      RETURN(0);
      return 0;
    }
    else if(addr[0] == '\0' && wmData->quietMode == FALSE)
    {
      // set the TO Field active and go back
      if(winOpen == TRUE)
        set(data->RG_PAGE, MUIA_Group_ActivePage, 2);

      set(obj, MUIA_Window_ActiveObject, data->ST_FROM);

      if(MUI_Request(G->App, obj, 0, NULL, tr(MSG_WR_NOSENDERREQGAD), tr(MSG_WR_ERRORNOSENDER)) == 0)
      {
        RETURN(0);
        return 0;
      }
    }
    else
      comp.From = addr;

    // then we check the CC string gadget
    addr = (char *)DoMethod(data->ST_CC, MUIM_Recipientstring_Resolve, MUIF_Recipientstring_Resolve_NoValid);
    if(addr == NULL)
    {
      ER_NewError(tr(MSG_ER_AliasNotFound), (STRPTR)xget(data->ST_CC, MUIA_String_Contents));

      if(winOpen == TRUE)
        set(data->RG_PAGE, MUIA_Group_ActivePage, 2);

      set(obj, MUIA_Window_ActiveObject, data->ST_CC);

      RETURN(0);
      return 0;
    }
    else if(addr[0] != '\0')
      comp.MailCC = addr;

    // then we check the BCC string gadget
    addr = (char *)DoMethod(data->ST_BCC, MUIM_Recipientstring_Resolve, MUIF_Recipientstring_Resolve_NoValid);
    if(addr == NULL)
    {
      ER_NewError(tr(MSG_ER_AliasNotFound), (STRPTR)xget(data->ST_BCC, MUIA_String_Contents));

      if(winOpen == TRUE)
        set(data->RG_PAGE, MUIA_Group_ActivePage, 2);

      set(obj, MUIA_Window_ActiveObject, data->ST_BCC);

      RETURN(0);
      return 0;
    }
    else if(addr[0] != '\0')
      comp.MailBCC = addr;

    // then we check the ReplyTo string gadget
    addr = (char *)DoMethod(data->ST_REPLYTO, MUIM_Recipientstring_Resolve, MUIF_Recipientstring_Resolve_NoValid);
    if(addr == NULL)
    {
      ER_NewError(tr(MSG_ER_AliasNotFound), (STRPTR)xget(data->ST_REPLYTO, MUIA_String_Contents));

      if(winOpen == TRUE)
        set(data->RG_PAGE, MUIA_Group_ActivePage, 2);

      set(obj, MUIA_Window_ActiveObject, data->ST_REPLYTO);

      RETURN(0);
      return 0;
    }
    else if(addr[0] != '\0')
      comp.ReplyTo = addr;

    comp.ExtHeader = (char *)xget(data->ST_EXTHEADER, MUIA_String_Contents);

    // In-Reply-To / References
    comp.inReplyToMsgID = wmData->inReplyToMsgID;
    comp.references = wmData->references;

    comp.Importance = 1-GetMUICycle(data->CY_IMPORTANCE);
    comp.RequestMDN = GetMUICheck(data->CH_MDN);
    comp.Signature = GetMUIRadio(data->RA_SIGNATURE);

    if((comp.Security = GetMUIRadio(data->RA_SECURITY)) == SEC_DEFAULTS &&
       SetDefaultSecurity(&comp) == FALSE)
    {
      RETURN(0);
      return 0;
    }

    // find the selected charset and default to the global one if it
    // could not be found
    if((wmData->charset = CodesetsFind((char *)xget(data->PO_CHARSET, MUIA_Text_Contents),
                                   CSA_CodesetList,       G->codesetsList,
                                   CSA_FallbackToDefault, FALSE,
                                   TAG_DONE)) == NULL)
    {
      wmData->charset = G->writeCharset;
    }

    comp.DelSend = GetMUICheck(data->CH_DELSEND);
    comp.UserInfo = GetMUICheck(data->CH_ADDINFO);

    numAttachments = xget(data->LV_ATTACH, MUIA_NList_Entries);

    // we execute the POSTWRITE macro right before writing out
    // the message because the postwrite macro may want to modify the
    // text in the editor beforehand.
    MA_StartMacro(MACRO_POSTWRITE, itoa(data->windowNumber));

    // export the text of our texteditor to a file
    DoMethod(data->TE_EDIT, MUIM_MailTextEdit_SaveToFile, data->wmData->filename);

    // build the whole mail part list including
    // the attachments
    if((comp.FirstPart = BuildPartsList(wmData)) == NULL)
    {
      RETURN(0);
      return 0;
    }
  }
  else
    MA_StartMacro(MACRO_POSTWRITE, itoa(data->windowNumber));

  // now we check how the new mail file should be named
  // or created off.
  switch(wmData->mode)
  {
    case NMM_EDIT:
    {
      if(wmData->refMail != NULL && MailExists(wmData->refMail, NULL) == TRUE)
      {
        GetMailFile(newMailFile, sizeof(newMailFile), outfolder, wmData->refMail);
        break;
      }
    }
    // continue

    case NMM_EDITASNEW:
      wmData->mode = NMM_NEW;
    // continue

    default:
      MA_NewMailFile(outfolder, newMailFile, sizeof(newMailFile));
    break;
  }

  // now open the new mail file for write operations
  if(newMailFile[0] != '\0' &&
     (comp.FH = fopen(newMailFile, "w")) != NULL)
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

      RETURN(0);
      return 0;
    }

    fclose(comp.FH);
    comp.FH = NULL;

    // stop any pending file notification.
    if(wmData->fileNotifyActive == TRUE)
    {
      EndNotify(wmData->notifyRequest);
      wmData->fileNotifyActive = FALSE;
    }

    if((email = MA_ExamineMail(outfolder, FilePart(newMailFile), C->EmailCache > 0 ? TRUE : FALSE)) != NULL)
    {
      email->Mail.sflags = stat;

      if((newMail = AddMailToList(&email->Mail, outfolder)) != NULL)
      {
        // Now we have to check whether we have to add the To & CC addresses
        // to the emailCache
        if(C->EmailCache > 0)
        {
          DoMethod(_app(obj), MUIM_YAMApplication_AddToEmailCache, &newMail->To);

          // if this mail has more than one recipient we have to add the others too
          if(isMultiRCPTMail(newMail))
          {
            int j;

            for(j = 0; j < email->NoSTo; j++)
              DoMethod(_app(obj), MUIM_YAMApplication_AddToEmailCache, &email->STo[j]);

            for(j = 0; j < email->NoCC; j++)
              DoMethod(_app(obj), MUIM_YAMApplication_AddToEmailCache, &email->CC[j]);

            for(j = 0; j < email->NoBCC; j++)
              DoMethod(_app(obj), MUIM_YAMApplication_AddToEmailCache, &email->BCC[j]);
          }
        }

        if(FO_GetCurrentFolder() == outfolder)
          DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_NList_InsertSingle, newMail, MUIV_NList_Insert_Sorted);

        MA_UpdateMailFile(newMail);

        // if this write operation was an edit mode
        // we have to check all existing readmail objects for
        // references and update them accordingly.
        if(wmData->mode == NMM_EDIT && wmData->refMail != NULL)
        {
          struct Node *curNode;

          // now we search through our existing readMailData
          // objects and see some of them are pointing to the old mail
          // and if so we signal them to display the new revised mail instead
          IterateList(&G->readMailDataList, curNode)
          {
            struct ReadMailData *rmData = (struct ReadMailData *)curNode;

            if(rmData->mail == wmData->refMail)
            {
              if(rmData->readWindow)
                DoMethod(rmData->readWindow, MUIM_ReadWindow_ReadMail, newMail);
              else if(rmData->readMailGroup)
                DoMethod(rmData->readMailGroup, MUIM_ReadMailGroup_ReadMail, newMail);
            }
          }

          RemoveMailFromList(wmData->refMail, TRUE, TRUE);
          wmData->refMail = newMail;
        }
      }

      // cleanup the email structure
      MA_FreeEMailStruct(email);
    }

    if(wmData->mode != NMM_NEW)
    {
      struct MailList *mlist;

      if(wmData->refMailList != NULL)
      {
        mlist = CloneMailList(wmData->refMailList);
      }
      else if(wmData->refMail != NULL)
      {
        if((mlist = CreateMailList()) != NULL)
          AddNewMailNode(mlist, wmData->refMail);
      }
      else
        mlist = NULL;

      if(mlist != NULL)
      {
        struct MailNode *mnode;

        ForEachMailNode(mlist, mnode)
        {
          struct Mail *mail = mnode->mail;

          if(mail != NULL && !isVirtualMail(mail) && mail->Folder != NULL &&
             !isOutgoingFolder(mail->Folder) && !isSentFolder(mail->Folder))
          {
            // process MDN notifications
            if(hasStatusNew(mail) || !hasStatusRead(mail))
              RE_ProcessMDN(MDN_MODE_DISPLAY, mail, FALSE, wmData->quietMode);

            switch(wmData->mode)
            {
              case NMM_REPLY:
              {
                setStatusToReplied(mail);
                DisplayStatistics(mail->Folder, FALSE);
              }
              break;

              case NMM_FORWARD:
              case NMM_FORWARD_ATTACH:
              case NMM_FORWARD_INLINE:
              case NMM_BOUNCE:
              {
                setStatusToForwarded(mail);
                DisplayStatistics(mail->Folder, FALSE);
              }
              break;

              default:
                // nothing
              break;
            }
          }
        }

        DeleteMailList(mlist);
      }
    }

    // if requested we make sure we also
    // output a log entry about the write operation
    switch(wmData->mode)
    {
      case NMM_NEW:
      case NMM_EDITASNEW:
        AppendToLogfile(LF_ALL, 10, tr(MSG_LOG_Creating), AddrName(newMail->To), newMail->Subject, numAttachments);
      break;

      case NMM_REPLY:
      {
        struct MailNode *mnode = FirstMailNode(wmData->refMailList);

        if(mnode != NULL)
          AppendToLogfile(LF_ALL, 11, tr(MSG_LOG_Replying), AddrName(mnode->mail->From), mnode->mail->Subject);
        else
          AppendToLogfile(LF_ALL, 11, tr(MSG_LOG_Replying), "<unknown>", "<unknown>");
      }
      break;

      case NMM_FORWARD:
      case NMM_FORWARD_ATTACH:
      case NMM_FORWARD_INLINE:
      {
        struct MailNode *mnode = FirstMailNode(wmData->refMailList);

        if(mnode != NULL)
          AppendToLogfile(LF_ALL, 12, tr(MSG_LOG_Forwarding), AddrName(mnode->mail->From), mnode->mail->Subject, AddrName(newMail->To));
        else
          AppendToLogfile(LF_ALL, 12, tr(MSG_LOG_Forwarding), "<unknown>", "<unknown>", AddrName(newMail->To));
      }
      break;

      case NMM_BOUNCE:
      {
        if(wmData->refMail)
          AppendToLogfile(LF_ALL, 13, tr(MSG_LOG_Bouncing), AddrName(wmData->refMail->From), wmData->refMail->Subject, AddrName(newMail->To));
        else
          AppendToLogfile(LF_ALL, 13, tr(MSG_LOG_Bouncing), "<unknown>", "<unknown>", AddrName(newMail->To));
      }
      break;

      case NMM_EDIT:
      {
        AppendToLogfile(LF_ALL, 14, tr(MSG_LOG_Editing), AddrName(newMail->From), AddrName(newMail->To), newMail->Subject);
      }
      break;

      case NMM_SAVEDEC:
        // not used
      break;
    }
  }
  else
     ER_NewError(tr(MSG_ER_CreateMailError));

  FreePartsList(comp.FirstPart);

  // cleanup certain references
  wmData->refMail = NULL;

  if(wmData->refMailList != NULL)
  {
    DeleteMailList(wmData->refMailList);
    wmData->refMailList = NULL;
  }

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

  // now we make sure we immediately send out the mail.
  if(mode == WRITE_SEND && newMail != NULL)
  {
    struct MailList *mlist;

    if((mlist = CreateMailList()) != NULL)
    {
      if(AddNewMailNode(mlist, newMail) != NULL)
      {
        struct MailServerNode *msn;
        BOOL mailSent = FALSE;

        set(obj, MUIA_Window_Open, FALSE);

        if((msn = GetMailServer(&C->mailServerList, MST_SMTP, 0)) != NULL)
        {
          if(hasServerInUse(msn) == FALSE)
          {
            // mark the server as "in use"
            SET_FLAG(msn->flags, MSF_IN_USE);

            mailSent = DoAction(TA_SendMails, TT_SendMails_MailServer, msn,
                                              TT_SendMails_Mails, mlist,
                                              TT_SendMails_Mode, SENDMAIL_ACTIVE_USER,
                                              TAG_DONE);
            if(mailSent == FALSE)
              CLEAR_FLAG(msn->flags, MSF_IN_USE);
          }
        }

        if(mailSent == FALSE)
          DeleteMailList(mlist);
      }
    }
  }

  // make sure to dispose the window data by calling
  // CloseWriteWindowHook() as soon as this method is finished
  DoMethod(G->App, MUIM_Application_PushMethod, G->App, 3, MUIM_CallHook, &CloseWriteWindowHook, data->wmData);

  // update the statistics of the outgoing folder
  DisplayStatistics(outfolder, TRUE);

  RETURN(0);
  return 0;
}

///
/// DECLARE(DroppedFile)
// Handles drag&drop of a file which was dropped on the
// dockyicon or appicon
DECLARE(DroppedFile) // STRPTR fileName
{
  GETDATA;
  int mode;

  ENTER();

  mode = xget(data->RG_PAGE, MUIA_Group_ActivePage);
  if(mode == 0)
  {
    FILE *fh;

    if((fh = fopen(msg->fileName, "r")) != NULL)
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

  if(mode == 0)
  {
    char *text;

    if((text = TransformText(msg->fileName, ED_INSERT, "")) != NULL)
    {
      DoMethod(data->TE_EDIT, MUIM_TextEditor_InsertText, text);
      free(text);
    }
    else
      DoMethod(obj, MUIM_WriteWindow_AddAttachment, msg->fileName, NULL, FALSE);
  }
  else
    DoMethod(obj, MUIM_WriteWindow_AddAttachment, msg->fileName, NULL, FALSE);

  RETURN(0);
  return 0;
}

///
/// DECLARE(DoAutoSave)
// check and perform an autosave of the mail text
DECLARE(DoAutoSave)
{
  GETDATA;

  ENTER();

  if(data->wmData->mode != NMM_BOUNCE)
  {
    char fileName[SIZE_PATHFILE];

    // do the autosave only if something was modified
    if(xget(data->TE_EDIT, MUIA_TextEditor_HasChanged) == TRUE)
    {
      if(DoMethod(data->TE_EDIT, MUIM_MailTextEdit_SaveToFile, WR_AutoSaveFile(data->windowNumber, fileName, sizeof(fileName))) == TRUE)
      {
        // we just saved the mail text, so it is no longer modified
        set(data->TE_EDIT, MUIA_TextEditor_HasChanged, FALSE);

        // we must remember if the mail was automatically saved, since the editor object cannot
        // tell about changes anymore if they don't happen from now on.
        data->autoSaved = TRUE;

        D(DBF_MAIL, "saved mail text of write window #%d in autosave file '%s'", data->windowNumber, fileName);
      }
      else
        W(DBF_MAIL, "EditorToFile failed");
    }
    else
      D(DBF_MAIL, "No changes found in editor, no need to save an autosave file");
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(CancelAction)
// CancelAction - User clicked the Cancel button
DECLARE(CancelAction)
{
  GETDATA;
  BOOL discard = TRUE;

  ENTER();

  if(data->wmData->mode != NMM_BOUNCE && data->wmData->quietMode == FALSE)
  {
    // ask the user what to do if the mail text was modified
    if(xget(data->TE_EDIT, MUIA_TextEditor_HasChanged) == TRUE || data->autoSaved == TRUE)
    {
      switch(MUI_Request(G->App, obj, 0, NULL, tr(MSG_WR_DiscardChangesGad), tr(MSG_WR_DiscardChanges)))
      {
        case 0:
        {
          // cancel
          discard = FALSE;
        }
        break;

        case 1:
        {
          // send later
          DoMethod(obj, MUIM_WriteWindow_ComposeMail, WRITE_QUEUE);
          discard = FALSE;
        }
        break;

        case 2:
        {
          // discard
        }
        break;
      }
    }
  }

  // check if we have to close/discard the window
  // However, please note that because we do kill the window upon closing it
  // we have to use MUIM_Application_PushMethod instead of calling the CloseWriteWindowHook
  // directly
  if(discard == TRUE)
    DoMethod(G->App, MUIM_Application_PushMethod, G->App, 3, MUIM_CallHook, &CloseWriteWindowHook, data->wmData);

  RETURN(0);
  return 0;
}

///
/// DECLARE(MailFileModified)
//
DECLARE(MailFileModified)
{
  GETDATA;
  struct DateStamp currentFileChangeTime;

  ENTER();

  // First we have to check if the date stamp of the file really did change, because notifications
  // are triggered by *any* change, i.e. setting a comment, changing the protection flags, changing
  // the contents. Since some editors (CubicIDE for example) do not only change the contents, but
  // also modify the protection bits and the file comment as well, we may receive more than just one
  // notification for the actual change of contents.
  if(ObtainFileInfo(data->wmData->filename, FI_DATE, &currentFileChangeTime) == TRUE)
  {
    if(CompareDates(&data->wmData->lastFileChangeTime, &currentFileChangeTime) > 0)
    {
      BOOL keep = FALSE;

      D(DBF_UTIL, "received notification that the tempfile of write window %ld have changed [%s]", data->windowNumber, data->wmData->filename);

      // check if the content of the TextEditor.mcc gadget changed
      // as well and if so warn the user
      if(xget(data->TE_EDIT, MUIA_TextEditor_HasChanged) == TRUE)
      {
        // warn the user that both the tempfile and the content of the TextEditor.mcc
        // changed.
        if(MUI_Request(G->App, obj, 0L, tr(MSG_FCHANGE_WARN_TITLE),
                                        tr(MSG_YesNoReq),
                                        tr(MSG_FCHANGE_WARN), data->windowNumber+1) == 0)
        {
          // cancel / keep old text
          keep = TRUE;
        }
      }

      if(keep == FALSE)
      {
        // let the TextEditor.mcc load the modified file and mark it as changed again
        DoMethod(obj, MUIM_WriteWindow_ReloadText, TRUE);

        // remember this new date stamp
        memcpy(&data->wmData->lastFileChangeTime, &currentFileChangeTime, sizeof(data->wmData->lastFileChangeTime));
      }
    }
  }

  RETURN(0);
  return 0;
}

///

