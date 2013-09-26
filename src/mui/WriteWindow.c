/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2013 YAM Open Source Team

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

#include <ctype.h>
#include <string.h>

#include <proto/codesets.h>
#include <proto/dos.h>
#include <proto/muimaster.h>
#include <proto/wb.h>
#include <dos/notify.h>
#include <libraries/gadtools.h>
#include <libraries/iffparse.h>
#include <mui/BetterString_mcc.h>
#include <mui/NBalance_mcc.h>
#include <mui/NList_mcc.h>
#include <mui/NListtree_mcc.h>
#include <mui/NListview_mcc.h>
#include <mui/TextEditor_mcc.h>
#include <mui/TheBar_mcc.h>
#include <workbench/startup.h>

#include "SDI_hook.h"

#include "YAM.h"
#include "YAM_error.h"
#include "YAM_global.h"
#include "YAM_glossarydisplay.h"
#include "YAM_mainFolder.h"

#include "mime/uucode.h"
#include "mui/AddressBookWindow.h"
#include "mui/CodesetPopup.h"
#include "mui/IdentityChooser.h"
#include "mui/MailTextEdit.h"
#include "mui/MimeTypePopup.h"
#include "mui/ReadMailGroup.h"
#include "mui/ReadWindow.h"
#include "mui/RecipientString.h"
#include "mui/SearchTextWindow.h"
#include "mui/SignatureChooser.h"
#include "mui/WriteAttachmentList.h"
#include "mui/WriteWindowToolbar.h"
#include "mui/YAMApplication.h"
#include "tcp/smtp.h"

#include "Busy.h"
#include "Config.h"
#include "DynamicString.h"
#include "FileInfo.h"
#include "FolderList.h"
#include "Locale.h"
#include "Logfile.h"
#include "MailList.h"
#include "MailServers.h"
#include "MUIObjects.h"
#include "ParseEmail.h"
#include "Requesters.h"
#include "Signature.h"
#include "Threads.h"
#include "UserIdentity.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *RG_PAGE;
  Object *LB_TO;
  Object *ST_TO;
  Object *GR_TO;
  Object *LB_SUBJECT;
  Object *ST_SUBJECT;
  Object *TX_POSI;
  Object *TE_EDIT;
  Object *TO_TOOLBAR;
  Object *LV_ATTACH;
  Object *GR_ATTACH_TINY;
  Object *LV_ATTACH_TINY;
  Object *BT_ADD;
  Object *BT_ADDPACK;
  Object *BT_REMOVE;
  Object *BT_RENAME;
  Object *BT_DISPLAY;
  Object *RA_ENCODING;
  Object *CY_CTYPE;
  Object *PO_CTYPE;
  Object *ST_DESC;
  Object *GR_HEADER;
  Object *LB_CC;
  Object *ST_CC;
  Object *GR_CC;
  Object *MN_CC;
  Object *LB_BCC;
  Object *ST_BCC;
  Object *GR_BCC;
  Object *MN_BCC;
  Object *LB_FROM;
  Object *CY_FROM;
  Object *LB_REPLYTO;
  Object *ST_REPLYTO;
  Object *GR_REPLYTO;
  Object *MN_REPLYTO;
  Object *LB_FROM_OVERRIDE;
  Object *ST_FROM_OVERRIDE;
  Object *GR_FROM_OVERRIDE;
  Object *ST_EXTHEADER;
  Object *CH_DELSEND;
  Object *CH_MDN;
  Object *CH_ADDINFO;
  Object *CY_IMPORTANCE;
  Object *CY_SECURITY;
  Object *CH_DEFSECURITY;
  Object *CY_SIGNATURE;
  Object *BT_SAVEASDRAFT;
  Object *BT_QUEUE;
  Object *BT_SEND;
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
  Object *PO_CODESET;
  Object *MI_SIGNATURE;
  Object *MI_SIGNATURES[MAXSIG_MENU];

  struct WriteMailData *wmData; // ptr to write mail data structure
  struct MsgPort *notifyPort;
  struct NotifyRequest *notifyRequest;

  int windowNumber; // the unique window number
  BOOL autoSaved;   // was this mail automatically saved?
  BOOL mailModified; // was anything modified?

  BOOL useFixedFont;   // use a fixed font for displaying the mail
  BOOL useTextColors;  // use Textcolors for displaying the mail
  BOOL useTextStyles;  // use Textstyles for displaying the mail

  BOOL fromRcptHidden;         // TRUE if From: identity chooser is hidden
  BOOL fromOverrideRcptHidden; // TRUE if From: recipient field is hidden
  BOOL ccRcptHidden;           // TRUE if CC: recipient field is hidden
  BOOL bccRcptHidden;          // TRUE if BCC: recipient field is hidden
  BOOL replyToRcptHidden;      // TRUE if Reply-To: recipient field is hidden

  char cursorPos[SIZE_SMALL];
  char windowTitle[SIZE_SUBJECT+1]; // string for the title text of the window
  char screenTitle[SIZE_LARGE];     // string for the title text of the screen
  char windowNumberStr[SIZE_SMALL]; // the unique window number as a string
};
*/

/* EXPORT
enum RcptType
{
  MUIV_WriteWindow_RcptType_From = 0,
  MUIV_WriteWindow_RcptType_FromOverride,
  MUIV_WriteWindow_RcptType_To,
  MUIV_WriteWindow_RcptType_CC,
  MUIV_WriteWindow_RcptType_BCC,
  MUIV_WriteWindow_RcptType_ReplyTo,
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
  WMEN_IMPORT2,WMEN_SIGN0,WMEN_SIGN1,WMEN_SIGN2,WMEN_SIGN3,WMEN_SIGN4,WMEN_SIGN5,WMEN_SIGN6,WMEN_SIGN7,
  WMEN_SECUR0,WMEN_SECUR1,WMEN_SECUR2,WMEN_SECUR3,WMEN_SECUR4,WMEN_INSUUCODE,
  WMEN_SENDNOW,WMEN_QUEUE,WMEN_SAVEASDRAFT,WMEN_CLOSE,WMEN_SWITCH1,WMEN_SWITCH2,WMEN_SWITCH3,
  WMEN_FFONT,WMEN_TSTYLE,WMEN_TCOLOR,WMEN_CC,WMEN_BCC,WMEN_REPLYTO
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
static enum Encoding WhichEncodingForFile(const char *fname,
                                          const char *ctype,
                                          const struct MailServerNode *msn)
{
  // default to base64 encoding, as this is able to transport anything
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
          ++unsafechars; // count the number of non 7bit ASCII chars
        else if (c < 32 && c != '\t')
          ++binarychars; // count the number of chars used in binaries.

        // if we successfully scanned 4000 bytes out of the file and found enough
        // data we break out here. We have to at least find some longlines or
        // we have to scan the whole part.
        if(total > 4000 && longlines > 0)
          break;
      }

      fclose(fh);

      D(DBF_MIME, "EncodingTest [%s] t:%ld l:%ld b:%ld", fname, total, longlines, binarychars);

      // now that we analyzed the file we have to decide which encoding to take
      if(longlines != 0 || unsafechars != 0 || binarychars != 0)
      {
        BOOL isApplication = (strnicmp(ctype, "application/", 12) == 0);

        if(binarychars == 0 && isApplication == FALSE)
        {
          // no binary characters and no binary attachment so far
          if(longlines != 0 || unsafechars != 0 || hasServer8bit(msn) == FALSE)
          {
            // use quoted-printable if there are either long lines or
            // non-7bit ASCII characters or if the SMTP server does not
            // support 7bit characters
            encoding = ENC_QP;
          }
          else
          {
            // otherwise use 8bit encoding
            encoding = ENC_8BIT;
          }
        }
        else
        {
          // if we end up here we have a file with either binary characters or the part
          // to be encoded is a binary attachment.
          encoding = ENC_B64;
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
static BOOL SetDefaultSecurity(struct Compose *comp, const Object *win)
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
      struct ABookNode *ab=NULL;
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
        if(t == NULL)
          continue;

        if(SearchABook(&G->abook, t, ASM_ADDRESS|ASM_USER|ASM_COMPLETE, &ab) != 0)
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
              if(MUI_Request(_app(win), win, MUIF_NONE, tr(MSG_WR_INVALIDSECURITY_TITLE),
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
            int res = MUI_Request(_app(win), win, MUIF_NONE, NULL, tr(MSG_WR_SECURITYREQ_GADS),
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
/// BuildPartsList
//  Builds message parts from attachment list
static struct WritePart *BuildPartsList(struct WriteMailData *wmData, BOOL delTemp)
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
    p->EncType = WhichEncodingForFile(p->Filename, p->ContentType, wmData->identity->smtpServer);

    // now walk through our attachment list
    // and create additional parts
    for(i=0; ;i++)
    {
      struct Attach *att = NULL;
      struct WritePart *np = NULL;

      DoMethod(wmData->window, METHOD(GetAttachment), i, &att);
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
          np->EncType = WhichEncodingForFile(np->Filename, np->ContentType, wmData->identity->smtpServer);

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
        FreePartsList(first, delTemp);
        first = NULL;

        break;
      }
    }
  }

  RETURN(first);
  return first;
}

///
/// TransformText
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

    if((dest = calloc(size, 1)) != NULL)
    {
      FILE *fp;

      if((fp = fopen(source, "r")) != NULL)
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
/// FreeCompose
// free a compose structure
static void FreeCompose(struct Compose *comp)
{
  ENTER();

  if(comp->MailReplyTo != NULL)
    free(comp->MailReplyTo);
  if(comp->MailFollowupTo != NULL)
    free(comp->MailFollowupTo);

  LEAVE();
}

///

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  ULONG i;
  struct Data *data;
  struct Data *tmpData;
  static const char *rtitles[4] = { NULL, NULL, NULL, NULL };
  static const char *security[SEC_MAXDUMMY+1];
  static const char *priority[4];

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

  security[SEC_NONE]    = tr(MSG_WR_SecNone);
  security[SEC_SIGN]    = tr(MSG_WR_SecSign);
  security[SEC_ENCRYPT] = tr(MSG_WR_SecEncrypt);
  security[SEC_BOTH]    = tr(MSG_WR_SecBoth);
  security[SEC_DEFAULTS]= tr(MSG_WR_SecDefaults);
  security[SEC_MAXDUMMY]= NULL;

  priority[0] = tr(MSG_WR_ImpHigh);
  priority[1] = tr(MSG_WR_ImpNormal);
  priority[2] = tr(MSG_WR_ImpLow);
  priority[3] = NULL;

  // set some default values
  data->useFixedFont = C->UseFixedFontWrite;
  data->useTextColors = C->UseTextColorsWrite;
  data->useTextStyles = C->UseTextStylesWrite;

  // before we create all objects of this new write window we have to
  // check which number we can set for this window. Therefore we search in our
  // current WriteMailData list and check which number we can give this window
  i = 0;
  do
  {
    struct WriteMailData *wmData;
    BOOL found = FALSE;

    IterateList(&G->writeMailDataList, struct WriteMailData *, wmData)
    {
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
      D(DBF_GUI, "free write window number %ld found.", i);
      data->windowNumber = i;
      snprintf(data->windowNumberStr, sizeof(data->windowNumberStr), "%d", (int)i);

      break;
    }

    i++;
  }
  while(TRUE);

  // allocate the new writeMailData structure
  if((data->wmData = AllocWriteMailData()) != NULL)
  {
    Object *menuStripObject = NULL;
    Object *slider;
    struct TagItem *tags = inittags(msg);
    struct TagItem *tag;
    struct UserIdentityNode *uin;

    // check for some tags present at OM_NEW
    while((tag = NextTagItem((APTR)&tags)) != NULL)
    {
      switch(tag->ti_Tag)
      {
        case ATTR(Mode): data->wmData->mode = (enum NewMailMode)tag->ti_Data; break;
        case ATTR(Quiet): data->wmData->quietMode = (BOOL)tag->ti_Data; break;
      }
    }

    // set user identity node
    uin = data->wmData->identity;

    // if this write window is processed in NMM_REDIRECT mode we create
    // a slightly different graphical interface and hide all other things
    if(data->wmData->mode == NMM_REDIRECT)
    {
      // create a minimal menu strip for the redirect window which just consists
      // of the standard editing operations and shortcuts for the 4 buttons
      // in this window. Without this menu strip copying and pasteing would be
      // impossible, as the BetterString objects do NOT handle key presses
      // themselves anymore.
      // A list of all currently used shortcuts follows below for the full
      // menu.
      menuStripObject = MenustripObject,
        MenuChild, MenuObject,
          MUIA_Menu_Title, tr(MSG_WR_Text),
          MUIA_Menu_CopyStrings, FALSE,
          MenuChild, Menuitem(tr(MSG_WR_MSENDNOW), "S", TRUE, FALSE, WMEN_SENDNOW),
          MenuChild, Menuitem(tr(MSG_WR_MSENDLATER), "L", TRUE, FALSE, WMEN_QUEUE),
        //MenuChild, Menuitem(tr(MSG_WR_MSAVEASDRAFT), "H", TRUE, FALSE, WMEN_SAVEASDRAFT),
          MenuChild, Menuitem(tr(MSG_WR_MCLOSE), "W", TRUE, FALSE, WMEN_CLOSE),
        End,
        MenuChild, MenuObject,
          MUIA_Menu_Title, tr(MSG_WR_Edit),
          MUIA_Menu_CopyStrings, FALSE,
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

      if(menuStripObject != NULL)
      {
        obj = DoSuperNew(cl, obj,
          MUIA_Window_Title, "",
          MUIA_Window_ScreenTitle, "",
          MUIA_HelpNode, "Windows#Writewindow",
          MUIA_Window_ID, MAKE_ID('W','R','I','B'),
          MUIA_Window_AppWindow, FALSE,
          MUIA_Window_Menustrip, menuStripObject,
          WindowContents, VGroup,

            Child, data->GR_HEADER = ColGroup(2),
              Child, data->LB_FROM = Label(tr(MSG_WR_REDIRECT_FROM)),
              Child, data->CY_FROM = IdentityChooserObject,
                MUIA_ControlChar, ShortCut(tr(MSG_WR_REDIRECT_FROM)),
              End,

              Child, data->LB_FROM_OVERRIDE = HGroup,
                MUIA_Group_PageMode, TRUE,
                MUIA_HorizWeight, 0,
                Child, HSpace(-1),
                Child, Label(tr(MSG_WR_From)),
              End,
              Child, data->GR_FROM_OVERRIDE = MakeAddressField(&data->ST_FROM_OVERRIDE, NULL, NULL, ABM_FROM, data->windowNumber, AFF_EXTERNAL_SHORTCUTS),

              Child, data->LB_TO = Label(tr(MSG_WR_REDIRECT_TO)),
              Child, data->GR_TO = MakeAddressField(&data->ST_TO, tr(MSG_WR_REDIRECT_TO), MSG_HELP_WR_ST_TO, ABM_TO, data->windowNumber, AFF_ALLOW_MULTI|AFF_EXTERNAL_SHORTCUTS),

              Child, data->LB_CC = Label(tr(MSG_WR_REDIRECT_CC)),
              Child, data->GR_CC = MakeAddressField(&data->ST_CC, tr(MSG_WR_REDIRECT_CC), MSG_HELP_WR_ST_CC, ABM_CC, data->windowNumber, AFF_ALLOW_MULTI|AFF_EXTERNAL_SHORTCUTS),

              Child, data->LB_BCC = Label(tr(MSG_WR_REDIRECT_BCC)),
              Child, data->GR_BCC = MakeAddressField(&data->ST_BCC, tr(MSG_WR_REDIRECT_BCC), MSG_HELP_WR_ST_BCC, ABM_BCC, data->windowNumber, AFF_ALLOW_MULTI|AFF_EXTERNAL_SHORTCUTS),
            End,

            Child, ColGroup(2),
              Child, data->BT_SEND        = MakeButton(tr(MSG_WR_SENDNOW)),
              Child, data->BT_QUEUE       = MakeButton(tr(MSG_WR_SENDLATER)),
            //Child, data->BT_SAVEASDRAFT = MakeButton(tr(MSG_WR_SAVEASDRAFT)),
            End,

          End,
          TAG_MORE, inittags(msg));
      }

      if(obj != NULL)
      {
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_SENDNOW,     obj, 2, METHOD(ComposeMail), WRITE_SEND);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_QUEUE,       obj, 2, METHOD(ComposeMail), WRITE_QUEUE);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_SAVEASDRAFT, obj, 2, METHOD(ComposeMail), WRITE_DRAFT);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_CLOSE,       obj, 1, METHOD(CancelAction));
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_CUT,         obj, 2, METHOD(EditActionPerformed), EA_CUT);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_COPY,        obj, 2, METHOD(EditActionPerformed), EA_COPY);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_PASTE,       obj, 2, METHOD(EditActionPerformed), EA_PASTE);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_DELETE,      obj, 2, METHOD(EditActionPerformed), EA_DELETE);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_SELECTALL,   obj, 2, METHOD(EditActionPerformed), EA_SELECTALL);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_SELECTNONE,  obj, 2, METHOD(EditActionPerformed), EA_SELECTNONE);
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
      //  H   Save mail as draft (reserved by YAM.cd)
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
      //  W   Close window (WMEN_CLOSE)
      //  X   reserved for 'Cut' operation (WMEN_CUT)
      //  Y   reserved for 'Redo' operation (WMEN_REDO)
      //  Z   reserved for 'Undo' operation (WMEN_UNDO)
      //  1   Switch to Message view (WMEN_SWITCH1)
      //  2   Switch to Attachment view (WMEN_SWITCH2)
      //  3   Switch to Options view (WMEN_SWITCH3)
      //  4   Show/Hide Cc Address-Field (WMEN_CC)
      //  5   Show/Hide Bcc Address-Field (WMEN_BCC)
      //  6   Show/Hide Reply-To Address-Field (WMEN_REPLYTO)
      //  0   Use no signature (WMEN_SIGN0)
      //  7   Use signature 1 (WMEN_SIGN1)
      //  8   Use signature 2 (WMEN_SIGN2)
      //  9   Use signature 3 (WMEN_SIGN3)

      menuStripObject = MenustripObject,
        MenuChild, MenuObject,
          MUIA_Menu_Title, tr(MSG_WR_Text),
          MUIA_Menu_CopyStrings, FALSE,
          MenuChild, Menuitem(tr(MSG_New), "N", TRUE, FALSE, WMEN_NEW),
          MenuChild, Menuitem(tr(MSG_Open), "O", TRUE, FALSE, WMEN_OPEN),
          MenuChild, MenuitemObject,
            MUIA_Menuitem_Title, tr(MSG_WR_InsertAs),
            MUIA_Menuitem_CopyStrings, FALSE,
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
          MenuChild, Menuitem(tr(MSG_WR_MSAVEASDRAFT), "H", TRUE, FALSE, WMEN_SAVEASDRAFT),
          MenuChild, Menuitem(tr(MSG_WR_MCLOSE), "W", TRUE, FALSE, WMEN_CLOSE),
        End,
        MenuChild, MenuObject,
          MUIA_Menu_Title, tr(MSG_WR_Edit),
          MUIA_Menu_CopyStrings, FALSE,
          MenuChild, Menuitem(tr(MSG_WR_MUndo), "Z", TRUE, FALSE, WMEN_UNDO),
          MenuChild, Menuitem(tr(MSG_WR_Redo), "Y", TRUE, FALSE, WMEN_REDO),
          MenuChild, MenuBarLabel,
          MenuChild, Menuitem(tr(MSG_WR_MCut), "X", TRUE, FALSE, WMEN_CUT),
          MenuChild, Menuitem(tr(MSG_WR_MCopy), "C", TRUE, FALSE, WMEN_COPY),
          MenuChild, Menuitem(tr(MSG_WR_MPaste), "V", TRUE, FALSE, WMEN_PASTE),
          MenuChild, MenuitemObject,
            MUIA_Menuitem_Title, tr(MSG_WR_PasteAs),
            MUIA_Menuitem_CopyStrings, FALSE,
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
            MUIA_Menuitem_CopyStrings, FALSE,
            MenuChild, data->MI_BOLD = MenuitemCheck(tr(MSG_WR_Bold), "B", TRUE, FALSE, TRUE, 0, WMEN_STYLE_BOLD),
            MenuChild, data->MI_ITALIC = MenuitemCheck(tr(MSG_WR_Italic), "I", TRUE, FALSE, TRUE, 0, WMEN_STYLE_ITALIC),
            MenuChild, data->MI_UNDERLINE = MenuitemCheck(tr(MSG_WR_Underlined), "U", TRUE, FALSE, TRUE, 0, WMEN_STYLE_UNDERLINE),
            MenuChild, data->MI_COLORED = MenuitemCheck(tr(MSG_WR_Colored), "K", TRUE, FALSE, TRUE, 0, WMEN_STYLE_COLORED),
          End,
          MenuChild, MenuitemObject,
            MUIA_Menuitem_Title,tr(MSG_WR_Emoticons),
            MUIA_Menuitem_CopyStrings, FALSE,
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
          MUIA_Menu_CopyStrings, FALSE,
          MenuChild, Menuitem(tr(MSG_WR_MAddFile), "R", TRUE, FALSE, WMEN_ADDFILE),
          MenuChild, Menuitem(tr(MSG_WR_AddCB), NULL, TRUE, FALSE, WMEN_ADDCLIP),
          MenuChild, Menuitem(tr(MSG_WR_AddKey), NULL, G->PGPVersion != 0, FALSE, WMEN_ADDPGP),
        End,
        MenuChild, MenuObject,
          MUIA_Menu_Title, tr(MSG_WR_VIEW),
          MUIA_Menu_CopyStrings, FALSE,
          MenuChild, Menuitem(tr(MSG_WR_MSWITCH_MSG), "1", TRUE, FALSE, WMEN_SWITCH1),
          MenuChild, Menuitem(tr(MSG_WR_MSWITCH_ATT), "2", TRUE, FALSE, WMEN_SWITCH2),
          MenuChild, Menuitem(tr(MSG_WR_MSWITCH_OPT), "3", TRUE, FALSE, WMEN_SWITCH3),
          MenuChild, data->MN_CC = MenuitemCheck(tr(MSG_WR_MCCADDRFIELD), "4", TRUE, C->ShowRcptFieldCC, TRUE, 0, WMEN_CC),
          MenuChild, data->MN_BCC = MenuitemCheck(tr(MSG_WR_MBCCADDRFIELD), "5", TRUE, C->ShowRcptFieldBCC, TRUE, 0, WMEN_BCC),
          MenuChild, data->MN_REPLYTO = MenuitemCheck(tr(MSG_WR_MREPLYTOADDRFIELD),"6", TRUE, C->ShowRcptFieldReplyTo, TRUE, 0, WMEN_REPLYTO),
        End,
        MenuChild, MenuObject,
          MUIA_Menu_Title, tr(MSG_Options),
          MUIA_Menu_CopyStrings, FALSE,
          MenuChild, data->MI_DELSEND = MenuitemCheck(tr(MSG_WR_MDelSend), NULL, TRUE, FALSE, TRUE, 0, WMEN_DELSEND),
          MenuChild, data->MI_MDN = MenuitemCheck(tr(MSG_WR_MReceipt), NULL, TRUE, FALSE, TRUE, 0, WMEN_MDN),
          MenuChild, data->MI_ADDINFO = MenuitemCheck(tr(MSG_WR_MAddInfo), NULL, TRUE, FALSE, TRUE, 0, WMEN_ADDINFO),
          MenuChild, MenuitemObject,
            MUIA_Menuitem_Title,tr(MSG_WR_MImportance),
            MenuChild, MenuitemCheck(priority[0], NULL, TRUE, FALSE, TRUE, 0x06, WMEN_IMPORT0),
            MenuChild, MenuitemCheck(priority[1], NULL, TRUE, TRUE,  TRUE, 0x05, WMEN_IMPORT1),
            MenuChild, MenuitemCheck(priority[2], NULL, TRUE, FALSE, TRUE, 0x03, WMEN_IMPORT2),
          End,
          MenuChild, data->MI_SIGNATURE = MenuitemObject,
            MUIA_Menuitem_Title,tr(MSG_CO_CrdSignature),
            MUIA_Menuitem_CopyStrings, FALSE,
          End,
          MenuChild, MenuitemObject,
            MUIA_Menuitem_Title,tr(MSG_CO_CrdSecurity),
            MUIA_Menuitem_CopyStrings, FALSE,
            MenuChild, MenuitemCheck(security[SEC_NONE], NULL, TRUE, FALSE, TRUE, 0x3E, WMEN_SECUR0),
            MenuChild, MenuitemCheck(security[SEC_SIGN], NULL, G->PGPVersion != 0, FALSE, TRUE, 0x3D, WMEN_SECUR1),
            MenuChild, MenuitemCheck(security[SEC_ENCRYPT], NULL, G->PGPVersion != 0, FALSE, TRUE, 0x3B, WMEN_SECUR2),
            MenuChild, MenuitemCheck(security[SEC_BOTH], NULL, G->PGPVersion != 0, FALSE, TRUE, 0x37, WMEN_SECUR3),
            MenuChild, MenuitemCheck(security[SEC_DEFAULTS], NULL, TRUE, TRUE, TRUE, 0x2F, WMEN_SECUR4),
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
          MUIA_Window_ScreenTitle, "",
          MUIA_HelpNode, "Windows#Writewindow",
          MUIA_Window_ID, MAKE_ID('W','R','W', data->windowNumber),
          MUIA_Window_AppWindow, TRUE,
          MUIA_Window_Menustrip, menuStripObject,
          WindowContents, VGroup,
            Child, data->RG_PAGE = RegisterGroup(rtitles),
              MUIA_CycleChain, TRUE,

              // Message
              Child, VGroup,
                MUIA_HelpNode, "Windows#WritewindowMessagesheet",

                Child, HGroup,
                  GroupSpacing(0),
                  Child, HGroup,
                    MUIA_HorizWeight, 75,

                    Child, data->GR_HEADER = ColGroup(2),
                      Child, data->LB_FROM = Label(tr(MSG_WR_From)),
                      Child, data->CY_FROM = IdentityChooserObject,
                        MUIA_ControlChar, ShortCut(tr(MSG_WR_From)),
                      End,

                      Child, data->LB_FROM_OVERRIDE = HGroup,
                        MUIA_Group_PageMode, TRUE,
                        MUIA_HorizWeight, 0,
                        Child, HSpace(-1),
                        Child, Label(tr(MSG_WR_From)),
                      End,
                      Child, data->GR_FROM_OVERRIDE = MakeAddressField(&data->ST_FROM_OVERRIDE, NULL, NULL, ABM_FROM, data->windowNumber, AFF_EXTERNAL_SHORTCUTS),

                      Child, data->LB_TO = Label(tr(MSG_WR_To)),
                      Child, data->GR_TO = MakeAddressField(&data->ST_TO, tr(MSG_WR_To), MSG_HELP_WR_ST_TO, ABM_TO, data->windowNumber, AFF_ALLOW_MULTI|AFF_EXTERNAL_SHORTCUTS),

                      Child, data->LB_CC = Label("_CC"),
                      Child, data->GR_CC = MakeAddressField(&data->ST_CC, tr(MSG_WR_CopyTo), MSG_HELP_WR_ST_CC, ABM_CC, data->windowNumber, AFF_ALLOW_MULTI|AFF_EXTERNAL_SHORTCUTS),

                      Child, data->LB_BCC = Label("_BCC"),
                      Child, data->GR_BCC = MakeAddressField(&data->ST_BCC, tr(MSG_WR_BlindCopyTo), MSG_HELP_WR_ST_BCC, ABM_BCC, data->windowNumber, AFF_ALLOW_MULTI|AFF_EXTERNAL_SHORTCUTS),

                      Child, data->LB_REPLYTO = Label(tr(MSG_WR_ReplyTo)),
                      Child, data->GR_REPLYTO = MakeAddressField(&data->ST_REPLYTO, tr(MSG_WR_ReplyTo), MSG_HELP_WR_ST_REPLYTO, ABM_REPLYTO, data->windowNumber, AFF_ALLOW_MULTI|AFF_EXTERNAL_SHORTCUTS),

                      Child, data->LB_SUBJECT = Label(tr(MSG_WR_Subject)),
                      Child, data->ST_SUBJECT = BetterStringObject,
                        StringFrame,
                        MUIA_BetterString_NoShortcuts, TRUE,
                        MUIA_String_MaxLen,            SIZE_SUBJECT,
                        MUIA_String_AdvanceOnCR,       TRUE,
                        MUIA_ControlChar,              ShortCut(tr(MSG_WR_Subject)),
                        MUIA_CycleChain,               TRUE,
                      End,
                    End,
                  End,
                  Child, data->GR_ATTACH_TINY = HGroup,
                    GroupSpacing(0),
                    MUIA_ShowMe, FALSE,
                    MUIA_HorizWeight, 25,
                    Child, NBalanceObject,
                      MUIA_Balance_Quiet, TRUE,
                    End,
                    Child, NListviewObject,
                      MUIA_CycleChain, TRUE,
                      MUIA_NListview_NList, data->LV_ATTACH_TINY = WriteAttachmentListObject,
                        MUIA_WriteAttachmentList_Tiny, TRUE,
                      End,
                    End,
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
                            MUIA_Text_Copy,     FALSE,
                            MUIA_Background,    MUII_RegisterBack,
                            MUIA_Frame,         MUIV_Frame_None,
                            MUIA_Font,          MUIV_Font_Tiny,
                          End,
                          Child, VSpace(0),
                        End,
                      End),
                End),

                Child, HGroup,
                  MUIA_HelpNode, "Windows#WritewindowInternaleditor",
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
                MUIA_HelpNode, "Windows#WritewindowAttachmentsheet",
                Child, NListviewObject,
                  MUIA_CycleChain, TRUE,
                  MUIA_NListview_NList, data->LV_ATTACH = WriteAttachmentListObject,
                  End,
                End,

                Child, ColGroup(5),
                  Child, data->BT_ADD     = MakeButton(tr(MSG_WR_Add)),
                  Child, data->BT_ADDPACK = MakeButton(tr(MSG_WR_AddPack)),
                  Child, data->BT_REMOVE  = MakeButton(tr(MSG_WR_REMOVE)),
                  Child, data->BT_RENAME  = MakeButton(tr(MSG_WR_RENAME)),
                  Child, data->BT_DISPLAY = MakeButton(tr(MSG_WR_Display)),
                End,

                Child, ColGroup(2),
                  Child, Label2(tr(MSG_WR_ContentType)),
                  Child, data->PO_CTYPE = MimeTypePopupObject,
                    MUIA_MimeTypePopup_ControlChar, ShortCut(tr(MSG_WR_ContentType)),
                  End,
                  Child, Label2(tr(MSG_WR_Description)),
                  Child, data->ST_DESC = BetterStringObject,
                    StringFrame,
                    MUIA_BetterString_NoShortcuts, TRUE,
                    MUIA_String_MaxLen,            SIZE_DEFAULT,
                    MUIA_String_AdvanceOnCR,       TRUE,
                    MUIA_ControlChar,              ShortCut(tr(MSG_WR_Description)),
                    MUIA_CycleChain,               TRUE,
                  End,
                End,
              End,

              // Options
              Child, VGroup,
                MUIA_HelpNode, "Windows#WritewindowOptionssheet",
                Child, ColGroup(2),
                  GroupFrameT(tr(MSG_Options)),

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
                  Child, data->PO_CODESET = CodesetPopupObject,
                    MUIA_CodesetPopup_ControlChar, tr(MSG_WR_CHARSET),
                  End,

                  Child, Label(tr(MSG_WR_Importance)),
                  Child, data->CY_IMPORTANCE = MakeCycle(priority, tr(MSG_WR_Importance)),

                  Child, Label(tr(MSG_WR_Signature)),
                  Child, data->CY_SIGNATURE = SignatureChooserObject,
                    MUIA_ControlChar, ShortCut(tr(MSG_WR_Signature)),
                  End,

                  Child, Label(tr(MSG_WR_PGPSECURITY)),
                  Child, data->CY_SECURITY = MakeCycle(security, tr(MSG_WR_PGPSECURITY)),

                  Child, HSpace(1),
                  Child, HBarT(tr(MSG_WR_SendOpt)), End,

                  Child, HSpace(1),
                  Child, MakeCheckGroup((Object **)&data->CH_DELSEND, tr(MSG_WR_DelSend)),

                  Child, HSpace(1),
                  Child, MakeCheckGroup((Object **)&data->CH_MDN,     tr(MSG_WR_Receipt)),

                  Child, HSpace(1),
                  Child, MakeCheckGroup((Object **)&data->CH_ADDINFO, tr(MSG_WR_AddInfo)),

                  Child, HVSpace,
                  Child, HVSpace,

                End,
              End,

            End,

            // Buttons
            Child, ColGroup(3),
              Child, data->BT_SEND        = MakeButton(tr(MSG_WR_SENDNOW)),
              Child, data->BT_QUEUE       = MakeButton(tr(MSG_WR_SENDLATER)),
              Child, data->BT_SAVEASDRAFT = MakeButton(tr(MSG_WR_SAVEASDRAFT)),
            End,
          End,

        TAG_MORE, inittags(msg));
      }
    }

    // check if object creation worked as expected
    if(obj != NULL)
    {
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
      // write window is not in REDIRECT mode
      if(data->wmData->mode != NMM_REDIRECT)
      {
        // prepare a first initial window title string
        snprintf(data->windowTitle, sizeof(data->windowTitle), "[%d] %s: ", data->windowNumber+1, tr(MSG_WR_WriteWT));

        // set the default window object
        xset(obj, MUIA_Window_DefaultObject, data->ST_TO,
                  MUIA_Window_ActiveObject,  data->ST_TO);

        // set the key focus attributes of the TO and SUBJECT gadgets
        xset(data->ST_TO, MUIA_BetterString_KeyDownFocus, data->ST_SUBJECT,
                          MUIA_RecipientString_ReplyToString, data->ST_REPLYTO);

        xset(data->ST_SUBJECT, MUIA_BetterString_KeyUpFocus, data->ST_TO,
                               MUIA_BetterString_KeyDownFocus, data->TE_EDIT);

        // set certain default check settings of the autospell and autowrap menu items
        set(data->MI_AUTOSPELL, MUIA_Menuitem_Checked, xget(data->TE_EDIT, MUIA_TextEditor_TypeAndSpell));
        set(data->MI_AUTOWRAP,  MUIA_Menuitem_Checked, xget(data->TE_EDIT, MUIA_TextEditor_WrapBorder) > 0);

        // set the codeset popupbutton string list contents
        nnset(data->PO_CODESET, MUIA_CodesetPopup_Codeset, C->DefaultWriteCodeset);

        // put the importance cycle gadget into the cycle group
        set(data->CY_IMPORTANCE, MUIA_Cycle_Active, TRUE);

        // disable certain GUI elements per default
        DoMethod(_app(obj), MUIM_MultiSet,  MUIA_Disabled, TRUE, data->PO_CTYPE,
                                                                 data->ST_DESC,
                                                                 data->BT_REMOVE,
                                                                 data->BT_RENAME,
                                                                 data->BT_DISPLAY,
                                                                 NULL);

        // set the help elements of our GUI gadgets
        SetHelp(data->ST_SUBJECT,     MSG_HELP_WR_ST_SUBJECT);
        SetHelp(data->BT_ADD,         MSG_HELP_WR_BT_ADD);
        SetHelp(data->BT_ADDPACK,     MSG_HELP_WR_BT_ADDPACK);
        SetHelp(data->BT_REMOVE,      MSG_HELP_WR_BT_REMOVE);
        SetHelp(data->BT_DISPLAY,     MSG_HELP_WR_BT_DISPLAY);
        SetHelp(data->PO_CTYPE,       MSG_HELP_WR_ST_CTYPE);
        SetHelp(data->ST_DESC,        MSG_HELP_WR_ST_DESC);
        SetHelp(data->ST_EXTHEADER,   MSG_HELP_WR_ST_EXTHEADER);
        SetHelp(data->CH_DELSEND,     MSG_HELP_WR_CH_DELSEND);
        SetHelp(data->CH_MDN,         MSG_HELP_WR_CH_RECEIPT);
        SetHelp(data->CH_ADDINFO,     MSG_HELP_WR_CH_ADDINFO);
        SetHelp(data->CY_IMPORTANCE,  MSG_HELP_WR_CY_IMPORTANCE);
        SetHelp(data->CY_SIGNATURE,   MSG_HELP_WR_CY_SIGNATURE);
        SetHelp(data->CY_SECURITY,    MSG_HELP_WR_CY_SECURITY);
        SetHelp(data->LV_ATTACH,      MSG_HELP_WR_LV_ATTACH);
        SetHelp(data->LV_ATTACH_TINY, MSG_HELP_WR_LV_ATTACH);

        // set the menuitem notifies
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_NEW,         data->TE_EDIT, 1, MUIM_TextEditor_ClearText);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_OPEN,        obj, 2, METHOD(EditorCmd), ED_OPEN);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_INSFILE,     obj, 2, METHOD(EditorCmd), ED_INSERT);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_INSQUOT,     obj, 2, METHOD(EditorCmd), ED_INSQUOT);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_INSALTQUOT,  obj, 2, METHOD(EditorCmd), ED_INSALTQUOT);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_INSROT13,    obj, 2, METHOD(EditorCmd), ED_INSROT13);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_INSUUCODE,   obj, 2, METHOD(EditorCmd), ED_INSUUCODE);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_SAVEAS,      obj, 1, METHOD(SaveTextAs));
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_EDIT,        obj, 1, METHOD(LaunchEditor));
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_SENDNOW,     obj, 2, METHOD(ComposeMail), WRITE_SEND);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_QUEUE,       obj, 2, METHOD(ComposeMail), WRITE_QUEUE);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_SAVEASDRAFT, obj, 2, METHOD(ComposeMail), WRITE_DRAFT);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_CLOSE,       obj, 1, METHOD(CancelAction));
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_CUT,         obj, 2, METHOD(EditActionPerformed), EA_CUT);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_COPY,        obj, 2, METHOD(EditActionPerformed), EA_COPY);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_PASTE,       obj, 2, METHOD(EditActionPerformed), EA_PASTE);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_DELETE,      obj, 2, METHOD(EditActionPerformed), EA_DELETE);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_PASQUOT,     obj, 2, METHOD(EditorCmd), ED_PASQUOT);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_PASALTQUOT,  obj, 2, METHOD(EditorCmd), ED_PASALTQUOT);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_PASROT13,    obj, 2, METHOD(EditorCmd), ED_PASROT13);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_SEARCH,      obj, 2, METHOD(Search), MUIF_NONE);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_SEARCHAGAIN, obj, 2, METHOD(Search), MUIF_ReadMailGroup_Search_Again);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_DICT,        MUIV_Notify_Application, 3, MUIM_CallHook, &DI_OpenHook, obj);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_UNDO,        obj, 2, METHOD(EditActionPerformed), EA_UNDO);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_REDO,        obj, 2, METHOD(EditActionPerformed), EA_REDO);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_ADDFILE,     obj, 2, METHOD(RequestAttachment), C->AttachDir);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_ADDCLIP,     obj, 1, METHOD(AddClipboard));
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_ADDPGP,      obj, 1, METHOD(AddPGPKey));
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_SELECTALL,   obj, 2, METHOD(EditActionPerformed), EA_SELECTALL);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_SELECTNONE,  obj, 2, METHOD(EditActionPerformed), EA_SELECTNONE);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_SWITCH1,     data->RG_PAGE, 3, MUIM_Set, MUIA_Group_ActivePage, 0);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_SWITCH2,     data->RG_PAGE, 3, MUIM_Set, MUIA_Group_ActivePage, 1);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_SWITCH3,     data->RG_PAGE, 3, MUIM_Set, MUIA_Group_ActivePage, 2);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_CC,          obj, 2, METHOD(MenuToggleRecipientObject), MUIV_WriteWindow_RcptType_CC);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_BCC,         obj, 2, METHOD(MenuToggleRecipientObject), MUIV_WriteWindow_RcptType_BCC);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_REPLYTO,     obj, 2, METHOD(MenuToggleRecipientObject), MUIV_WriteWindow_RcptType_ReplyTo);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_EMOT0,       data->TE_EDIT, 3, MUIM_TextEditor_InsertText, ":-)", MUIV_TextEditor_InsertText_Cursor);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_EMOT1,       data->TE_EDIT, 3, MUIM_TextEditor_InsertText, ":-|", MUIV_TextEditor_InsertText_Cursor);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_EMOT2,       data->TE_EDIT, 3, MUIM_TextEditor_InsertText, ":-(", MUIV_TextEditor_InsertText_Cursor);
        DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, WMEN_EMOT3,       data->TE_EDIT, 3, MUIM_TextEditor_InsertText, ";-)", MUIV_TextEditor_InsertText_Cursor);

        // catch MUIA_AppMessage with a hook so that we get notified
        // as soon as the user drops a WB icon on the pagegroup object
        DoMethod(data->RG_PAGE, MUIM_Notify, MUIA_AppMessage, MUIV_EveryTime, obj, 2, METHOD(HandleAppMessage), MUIV_TriggerValue);

        // set some notifications on the toolbar
        // in case it was generated
        if(data->TO_TOOLBAR != NULL)
        {
          // connect the buttons presses
          DoMethod(data->TO_TOOLBAR, MUIM_TheBar_Notify, TB_WRITE_EDITOR,    MUIA_Pressed, FALSE, obj, 1, METHOD(LaunchEditor));
          DoMethod(data->TO_TOOLBAR, MUIM_TheBar_Notify, TB_WRITE_INSERT,    MUIA_Pressed, FALSE, obj, 2, METHOD(EditorCmd), ED_INSERT);
          DoMethod(data->TO_TOOLBAR, MUIM_TheBar_Notify, TB_WRITE_CUT,       MUIA_Pressed, FALSE, data->TE_EDIT, 2, MUIM_TextEditor_ARexxCmd, "CUT");
          DoMethod(data->TO_TOOLBAR, MUIM_TheBar_Notify, TB_WRITE_COPY,      MUIA_Pressed, FALSE, data->TE_EDIT, 2, MUIM_TextEditor_ARexxCmd, "COPY");
          DoMethod(data->TO_TOOLBAR, MUIM_TheBar_Notify, TB_WRITE_PASTE,     MUIA_Pressed, FALSE, data->TE_EDIT, 2, MUIM_TextEditor_ARexxCmd, "PASTE");
          DoMethod(data->TO_TOOLBAR, MUIM_TheBar_Notify, TB_WRITE_UNDO,      MUIA_Pressed, FALSE, data->TE_EDIT, 2, MUIM_TextEditor_ARexxCmd, "UNDO");
          DoMethod(data->TO_TOOLBAR, MUIM_TheBar_Notify, TB_WRITE_BOLD,      MUIA_Selected, MUIV_EveryTime, obj, 3, METHOD(SetSoftStyle), SSM_BOLD, ORIGIN_TOOLBAR);
          DoMethod(data->TO_TOOLBAR, MUIM_TheBar_Notify, TB_WRITE_ITALIC,    MUIA_Selected, MUIV_EveryTime, obj, 3, METHOD(SetSoftStyle), SSM_ITALIC, ORIGIN_TOOLBAR);
          DoMethod(data->TO_TOOLBAR, MUIM_TheBar_Notify, TB_WRITE_UNDERLINE, MUIA_Selected, MUIV_EveryTime, obj, 3, METHOD(SetSoftStyle), SSM_UNDERLINE, ORIGIN_TOOLBAR);
          DoMethod(data->TO_TOOLBAR, MUIM_TheBar_Notify, TB_WRITE_COLORED,   MUIA_Selected, MUIV_EveryTime, obj, 3, METHOD(SetSoftStyle), SSM_COLOR, ORIGIN_TOOLBAR);
          DoMethod(data->TO_TOOLBAR, MUIM_TheBar_Notify, TB_WRITE_SEARCH,    MUIA_Pressed, FALSE, obj, 3, METHOD(Search), MUIF_NONE);

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
          DoMethod(data->TE_EDIT, MUIM_Notify, MUIA_TextEditor_CursorX, MUIV_EveryTime, obj, 1, METHOD(UpdateCursorPos));
          DoMethod(data->TE_EDIT, MUIM_Notify, MUIA_TextEditor_CursorY, MUIV_EveryTime, obj, 1, METHOD(UpdateCursorPos));
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

        DoMethod(data->MI_BOLD,        MUIM_Notify, MUIA_Menuitem_Checked, MUIV_EveryTime, obj, 3, METHOD(SetSoftStyle), SSM_BOLD, ORIGIN_MENU);
        DoMethod(data->MI_ITALIC,      MUIM_Notify, MUIA_Menuitem_Checked, MUIV_EveryTime, obj, 3, METHOD(SetSoftStyle), SSM_ITALIC, ORIGIN_MENU);
        DoMethod(data->MI_UNDERLINE,   MUIM_Notify, MUIA_Menuitem_Checked, MUIV_EveryTime, obj, 3, METHOD(SetSoftStyle), SSM_UNDERLINE, ORIGIN_MENU);
        DoMethod(data->MI_COLORED,     MUIM_Notify, MUIA_Menuitem_Checked, MUIV_EveryTime, obj, 3, METHOD(SetSoftStyle), SSM_COLOR, ORIGIN_MENU);

        DoMethod(data->RG_PAGE,        MUIM_Notify, MUIA_Group_ActivePage,                  0, MUIV_Notify_Window, 3, MUIM_Set, MUIA_Window_ActiveObject, data->TE_EDIT);
        DoMethod(data->RG_PAGE,        MUIM_Notify, MUIA_Group_ActivePage,                  1, MUIV_Notify_Window, 3, MUIM_Set, MUIA_Window_ActiveObject, data->LV_ATTACH);
        DoMethod(data->ST_SUBJECT,     MUIM_Notify, MUIA_String_Acknowledge,                MUIV_EveryTime, MUIV_Notify_Window, 3, MUIM_Set, MUIA_Window_ActiveObject, data->TE_EDIT);
        DoMethod(data->ST_SUBJECT,     MUIM_Notify, MUIA_String_Contents,                   MUIV_EveryTime, obj, 1, METHOD(UpdateWindowTitle));
        DoMethod(data->BT_ADD,         MUIM_Notify, MUIA_Pressed,                           FALSE,          obj, 2, METHOD(RequestAttachment), C->AttachDir);
        DoMethod(data->BT_ADDPACK,     MUIM_Notify, MUIA_Pressed,                           FALSE,          obj, 1, METHOD(AddArchive));
        DoMethod(data->BT_REMOVE,      MUIM_Notify, MUIA_Pressed,                           FALSE,          obj, 1, METHOD(RemoveAttachment));
        DoMethod(data->BT_RENAME,      MUIM_Notify, MUIA_Pressed,                           FALSE,          obj, 1, METHOD(RenameAttachment));
        DoMethod(data->BT_DISPLAY,     MUIM_Notify, MUIA_Pressed,                           FALSE,          obj, 1, METHOD(DisplayAttachment));
        DoMethod(data->LV_ATTACH,      MUIM_Notify, MUIA_NList_DoubleClick,                 MUIV_EveryTime, obj, 1, METHOD(DisplayAttachment));
        DoMethod(data->LV_ATTACH,      MUIM_Notify, MUIA_NList_Active,                      MUIV_EveryTime, obj, 1, METHOD(GetAttachmentEntry));
        DoMethod(data->LV_ATTACH_TINY, MUIM_Notify, MUIA_NList_DoubleClick,                 MUIV_EveryTime, obj, 1, METHOD(DisplayAttachment));
        DoMethod(data->LV_ATTACH_TINY, MUIM_Notify, MUIA_NList_Active,                      MUIV_EveryTime, obj, 1, METHOD(GetAttachmentEntry));
        DoMethod(data->PO_CTYPE,       MUIM_Notify, MUIA_MimeTypePopup_MimeTypeChanged,     MUIV_EveryTime, obj, 1, METHOD(PutAttachmentEntry));
        DoMethod(data->ST_DESC,        MUIM_Notify, MUIA_String_Contents,                   MUIV_EveryTime, obj, 1, METHOD(PutAttachmentEntry));
        DoMethod(data->CH_DELSEND,     MUIM_Notify, MUIA_Selected,                          MUIV_EveryTime, data->MI_DELSEND,        3, MUIM_Set,      MUIA_Menuitem_Checked, MUIV_TriggerValue);
        DoMethod(data->CH_MDN,         MUIM_Notify, MUIA_Selected,                          MUIV_EveryTime, data->MI_MDN,            3, MUIM_Set,      MUIA_Menuitem_Checked, MUIV_TriggerValue);
        DoMethod(data->CH_ADDINFO,     MUIM_Notify, MUIA_Selected,                          MUIV_EveryTime, data->MI_ADDINFO,        3, MUIM_Set,      MUIA_Menuitem_Checked, MUIV_TriggerValue);
        DoMethod(data->MI_AUTOSPELL,   MUIM_Notify, MUIA_Menuitem_Checked,                  MUIV_EveryTime, data->TE_EDIT,           3, MUIM_Set,      MUIA_TextEditor_TypeAndSpell, MUIV_TriggerValue);
        DoMethod(data->MI_AUTOWRAP,    MUIM_Notify, MUIA_Menuitem_Checked,                  TRUE,           data->TE_EDIT,           3, MUIM_Set,      MUIA_TextEditor_WrapBorder, C->EdWrapCol);
        DoMethod(data->MI_AUTOWRAP,    MUIM_Notify, MUIA_Menuitem_Checked,                  FALSE,          data->TE_EDIT,           3, MUIM_Set,      MUIA_TextEditor_WrapBorder, 0);
        DoMethod(data->MI_DELSEND,     MUIM_Notify, MUIA_Menuitem_Checked,                  MUIV_EveryTime, data->CH_DELSEND,        3, MUIM_Set,      MUIA_Selected, MUIV_TriggerValue);
        DoMethod(data->MI_MDN,         MUIM_Notify, MUIA_Menuitem_Checked,                  MUIV_EveryTime, data->CH_MDN,            3, MUIM_Set,      MUIA_Selected, MUIV_TriggerValue);
        DoMethod(data->MI_ADDINFO,     MUIM_Notify, MUIA_Menuitem_Checked,                  MUIV_EveryTime, data->CH_ADDINFO,        3, MUIM_Set,      MUIA_Selected, MUIV_TriggerValue);
        DoMethod(data->MI_FFONT,       MUIM_Notify, MUIA_Menuitem_Checked,                  MUIV_EveryTime, obj, 1, METHOD(StyleOptionsChanged));
        DoMethod(data->MI_TCOLOR,      MUIM_Notify, MUIA_Menuitem_Checked,                  MUIV_EveryTime, obj, 1, METHOD(StyleOptionsChanged));
        DoMethod(data->MI_TSTYLE,      MUIM_Notify, MUIA_Menuitem_Checked,                  MUIV_EveryTime, obj, 1, METHOD(StyleOptionsChanged));

        // set the notifies for the importance cycle gadget
        DoMethod(data->CY_IMPORTANCE, MUIM_Notify, MUIA_Cycle_Active,      0,              menuStripObject,         4, MUIM_SetUData, WMEN_IMPORT0, MUIA_Menuitem_Checked, TRUE);
        DoMethod(obj,                 MUIM_Notify, MUIA_Window_MenuAction, WMEN_IMPORT0,   data->CY_IMPORTANCE,     3, MUIM_Set,      MUIA_Cycle_Active, 0);
        DoMethod(data->CY_IMPORTANCE, MUIM_Notify, MUIA_Cycle_Active,      1,              menuStripObject,         4, MUIM_SetUData, WMEN_IMPORT1, MUIA_Menuitem_Checked, TRUE);
        DoMethod(obj,                 MUIM_Notify, MUIA_Window_MenuAction, WMEN_IMPORT1,   data->CY_IMPORTANCE,     3, MUIM_Set,      MUIA_Cycle_Active, 1);
        DoMethod(data->CY_IMPORTANCE, MUIM_Notify, MUIA_Cycle_Active,      2,              menuStripObject,         4, MUIM_SetUData, WMEN_IMPORT2, MUIA_Menuitem_Checked, TRUE);
        DoMethod(obj,                 MUIM_Notify, MUIA_Window_MenuAction, WMEN_IMPORT2,   data->CY_IMPORTANCE,     3, MUIM_Set,      MUIA_Cycle_Active, 2);

        // set the notifies for the security cycle gadget
        DoMethod(data->CY_SECURITY,   MUIM_Notify, MUIA_Cycle_Active,      0,              menuStripObject,         4, MUIM_SetUData, WMEN_SECUR0, MUIA_Menuitem_Checked, TRUE);
        DoMethod(obj,                 MUIM_Notify, MUIA_Window_MenuAction, WMEN_SECUR0,    data->CY_SECURITY,       3, MUIM_Set,      MUIA_Cycle_Active, 0);
        DoMethod(data->CY_SECURITY,   MUIM_Notify, MUIA_Cycle_Active,      1,              menuStripObject,         4, MUIM_SetUData, WMEN_SECUR1, MUIA_Menuitem_Checked, TRUE);
        DoMethod(obj,                 MUIM_Notify, MUIA_Window_MenuAction, WMEN_SECUR1,    data->CY_SECURITY,       3, MUIM_Set,      MUIA_Cycle_Active, 1);
        DoMethod(data->CY_SECURITY,   MUIM_Notify, MUIA_Cycle_Active,      2,              menuStripObject,         4, MUIM_SetUData, WMEN_SECUR2, MUIA_Menuitem_Checked, TRUE);
        DoMethod(obj,                 MUIM_Notify, MUIA_Window_MenuAction, WMEN_SECUR2,    data->CY_SECURITY,       3, MUIM_Set,      MUIA_Cycle_Active, 2);
        DoMethod(data->CY_SECURITY,   MUIM_Notify, MUIA_Cycle_Active,      3,              menuStripObject,         4, MUIM_SetUData, WMEN_SECUR3, MUIA_Menuitem_Checked, TRUE);
        DoMethod(obj,                 MUIM_Notify, MUIA_Window_MenuAction, WMEN_SECUR3,    data->CY_SECURITY,       3, MUIM_Set,      MUIA_Cycle_Active, 3);
        DoMethod(data->CY_SECURITY,   MUIM_Notify, MUIA_Cycle_Active,      4,              menuStripObject,         4, MUIM_SetUData, WMEN_SECUR4, MUIA_Menuitem_Checked, TRUE);
        DoMethod(obj,                 MUIM_Notify, MUIA_Window_MenuAction, WMEN_SECUR4,    data->CY_SECURITY,       3, MUIM_Set,      MUIA_Cycle_Active, 4);

        // set notify for signature cycle gadget
        DoMethod(data->CY_SIGNATURE, MUIM_Notify, MUIA_SignatureChooser_Signature, MUIV_EveryTime, obj, 1, METHOD(SignatureChanged));

        // tell the two attachment list which other object is to be kept in sync
        set(data->LV_ATTACH,      MUIA_WriteAttachmentList_SyncList, data->LV_ATTACH_TINY);
        set(data->LV_ATTACH_TINY, MUIA_WriteAttachmentList_SyncList, data->LV_ATTACH);

        // hide optional recipient string object depending on their
        // defaults
        if(C->ShowRcptFieldCC == FALSE)
          DoMethod(obj, METHOD(HideRecipientObject), MUIV_WriteWindow_RcptType_CC);
        if(C->ShowRcptFieldBCC == FALSE)
          DoMethod(obj, METHOD(HideRecipientObject), MUIV_WriteWindow_RcptType_BCC);
        if(C->ShowRcptFieldReplyTo == FALSE)
          DoMethod(obj, METHOD(HideRecipientObject), MUIV_WriteWindow_RcptType_ReplyTo);

        // update the available signatures first
        DoMethod(obj, METHOD(UpdateSignatures));
      }
      else
      {
        // prepare a first initial window title string
        strlcpy(data->windowTitle, tr(MSG_WR_REDIRECT_TITLE), sizeof(data->windowTitle));
      }

      // set the default window and screen title
      xset(obj, MUIA_Window_Title,       data->windowTitle,
                MUIA_Window_ScreenTitle, CreateScreenTitle(data->screenTitle, sizeof(data->screenTitle), data->windowTitle));

      // hide the override from address gadget also
      // in redirect mode
      if(C->OverrideFromAddress == FALSE)
        DoMethod(obj, METHOD(HideRecipientObject), MUIV_WriteWindow_RcptType_FromOverride);

      // set notify for identity cycle gadget
      DoMethod(data->CY_FROM, MUIM_Notify, MUIA_IdentityChooser_Identity, MUIV_EveryTime, obj, 2, METHOD(IdentityChanged), MUIV_TriggerValue);

      // make sure update the IdentityChooser state
      DoMethod(obj, METHOD(IdentityChanged), uin);

      // if we only have one identity we hide the identitychooser object
      if(xget(data->CY_FROM, MUIA_IdentityChooser_NumIdentities) < 2)
        DoMethod(obj, METHOD(HideRecipientObject), MUIV_WriteWindow_RcptType_From);

      // set some help text
      SetHelp(data->ST_TO,          MSG_HELP_WR_ST_TO);
      SetHelp(data->BT_QUEUE,       MSG_HELP_WR_BT_QUEUE);
      SetHelp(data->BT_SAVEASDRAFT, MSG_HELP_WR_BT_SAVEASDRAFT);
      SetHelp(data->BT_SEND,        MSG_HELP_WR_BT_SEND);
      SetHelp(data->PO_CODESET,     MSG_HELP_WR_PO_CHARSET);

      // declare the mail as modified if any of these objects reports a change
      DoMethod(data->ST_FROM_OVERRIDE, MUIM_Notify, MUIA_String_Contents,                   MUIV_EveryTime, obj, 3, MUIM_Set, ATTR(Modified), TRUE);
      DoMethod(data->ST_TO,            MUIM_Notify, MUIA_String_Contents,                   MUIV_EveryTime, obj, 3, MUIM_Set, ATTR(Modified), TRUE);
      DoMethod(data->ST_CC,            MUIM_Notify, MUIA_String_Contents,                   MUIV_EveryTime, obj, 3, MUIM_Set, ATTR(Modified), TRUE);
      DoMethod(data->ST_BCC,           MUIM_Notify, MUIA_String_Contents,                   MUIV_EveryTime, obj, 3, MUIM_Set, ATTR(Modified), TRUE);
      DoMethod(data->ST_REPLYTO,       MUIM_Notify, MUIA_String_Contents,                   MUIV_EveryTime, obj, 3, MUIM_Set, ATTR(Modified), TRUE);
      DoMethod(data->ST_EXTHEADER,     MUIM_Notify, MUIA_String_Contents,                   MUIV_EveryTime, obj, 3, MUIM_Set, ATTR(Modified), TRUE);
      DoMethod(data->PO_CTYPE,         MUIM_Notify, MUIA_MimeTypePopup_MimeTypeChanged,     MUIV_EveryTime, obj, 3, MUIM_Set, ATTR(Modified), TRUE);
      DoMethod(data->ST_DESC,          MUIM_Notify, MUIA_String_Contents,                   MUIV_EveryTime, obj, 3, MUIM_Set, ATTR(Modified), TRUE);
      DoMethod(data->PO_CODESET,       MUIM_Notify, MUIA_CodesetPopup_CodesetChanged,       MUIV_EveryTime, obj, 3, MUIM_Set, ATTR(Modified), TRUE);
      DoMethod(data->CY_IMPORTANCE,    MUIM_Notify, MUIA_Cycle_Active,                      MUIV_EveryTime, obj, 3, MUIM_Set, ATTR(Modified), TRUE);
      DoMethod(data->CY_SECURITY,      MUIM_Notify, MUIA_Cycle_Active,                      MUIV_EveryTime, obj, 3, MUIM_Set, ATTR(Modified), TRUE);
      DoMethod(data->CH_DELSEND,       MUIM_Notify, MUIA_Selected,                          MUIV_EveryTime, obj, 3, MUIM_Set, ATTR(Modified), TRUE);
      DoMethod(data->CH_MDN,           MUIM_Notify, MUIA_Selected,                          MUIV_EveryTime, obj, 3, MUIM_Set, ATTR(Modified), TRUE);
      DoMethod(data->CH_ADDINFO,       MUIM_Notify, MUIA_Selected,                          MUIV_EveryTime, obj, 3, MUIM_Set, ATTR(Modified), TRUE);

      // create a notify for changing the codeset
      DoMethod(data->PO_CODESET, MUIM_Notify, MUIA_CodesetPopup_Codeset, MUIV_EveryTime, obj, 2, METHOD(CodesetChanged), MUIV_TriggerValue);

      // set main window button notifies
      DoMethod(data->BT_SAVEASDRAFT, MUIM_Notify, MUIA_Pressed, FALSE, obj, 2, METHOD(ComposeMail), WRITE_DRAFT);
      DoMethod(data->BT_QUEUE,       MUIM_Notify, MUIA_Pressed, FALSE, obj, 2, METHOD(ComposeMail), WRITE_QUEUE);
      DoMethod(data->BT_SEND,        MUIM_Notify, MUIA_Pressed, FALSE, obj, 2, METHOD(ComposeMail), WRITE_SEND);

      // connect the closerequest attribute to the cancel action method so that
      // users might get informed of an eventually data loss
      DoMethod(obj, MUIM_Notify, MUIA_Window_CloseRequest, TRUE, obj, 1, METHOD(CancelAction));

      // prepare the temporary filename of that new write window
      snprintf(filename, sizeof(filename), "YAMw%08x-%d.txt", (unsigned int)FindTask(NULL), data->windowNumber+1);
      AddPath(data->wmData->filename, C->TempDir, filename, sizeof(data->wmData->filename));

      // set the global codeset as the default one
      data->wmData->codeset = G->writeCodeset;

      // Finally set up the notifications for external changes to the file being edited
      // if this is not a redirect window. We let the UserData point to this object to be
      // able to invoke a method from the main loop. However, we only do this for write
      // windows which are not opened in redirect mode
      if(data->wmData->mode != NMM_REDIRECT)
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

      if(data->wmData->mode != NMM_REDIRECT)
      {
        // finally set up the notifications for external changes to the file being edited if this is not a redirect window
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
      MA_StartMacro(MACRO_PREWRITE, data->windowNumberStr);

      // the mail is not modified yet
      data->mailModified = FALSE;
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

  if(data->wmData->mode != NMM_REDIRECT)
  {
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
  }

  // check the reference window ptr of the addressbook
  if(G->ABookWinObject != NULL && (LONG)xget(G->ABookWinObject, MUIA_AddressBookWindow_WindowNumber) == data->windowNumber)
    set(G->ABookWinObject, MUIA_AddressBookWindow_WindowNumber, -1);

  // we have to dispose certain object on our own
  // because they may be hidden by the user
  if(data->fromRcptHidden == TRUE)
  {
    MUI_DisposeObject(data->LB_FROM);
    MUI_DisposeObject(data->CY_FROM);
  }

  if(data->fromOverrideRcptHidden == TRUE)
  {
    MUI_DisposeObject(data->LB_FROM_OVERRIDE);
    MUI_DisposeObject(data->GR_FROM_OVERRIDE);
  }

  if(data->ccRcptHidden == TRUE)
  {
    MUI_DisposeObject(data->LB_CC);
    MUI_DisposeObject(data->GR_CC);
  }

  if(data->bccRcptHidden == TRUE)
  {
    MUI_DisposeObject(data->LB_BCC);
    MUI_DisposeObject(data->GR_BCC);
  }

  if(data->replyToRcptHidden == TRUE)
  {
    MUI_DisposeObject(data->LB_REPLYTO);
    MUI_DisposeObject(data->GR_REPLYTO);
  }

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

      case ATTR(Identity):
      {
        set(data->CY_FROM, MUIA_IdentityChooser_Identity, tag->ti_Data);

        // make the superMethod call ignore those tags
        tag->ti_Tag = TAG_IGNORE;
      }
      break;

      case ATTR(From):
      {
        setstring(data->ST_FROM_OVERRIDE, tag->ti_Data);

        // make the superMethod call ignore those tags
        tag->ti_Tag = TAG_IGNORE;
      }
      break;

      case ATTR(To):
      {
        setstring(data->ST_TO, tag->ti_Data);

        // make the superMethod call ignore those tags
        tag->ti_Tag = TAG_IGNORE;
      }
      break;

      case ATTR(CC):
      {
        char *str = (char *)tag->ti_Data;

        setstring(data->ST_CC, str);
        if(IsStrEmpty(str) == FALSE)
          DoMethod(obj, METHOD(ShowRecipientObject), MUIV_WriteWindow_RcptType_CC);

        // make the superMethod call ignore those tags
        tag->ti_Tag = TAG_IGNORE;
      }
      break;

      case ATTR(BCC):
      {
        char *str = (char *)tag->ti_Data;

        setstring(data->ST_BCC, str);
        if(IsStrEmpty(str) == FALSE)
          DoMethod(obj, METHOD(ShowRecipientObject), MUIV_WriteWindow_RcptType_BCC);

        // make the superMethod call ignore those tags
        tag->ti_Tag = TAG_IGNORE;
      }
      break;

      case ATTR(ReplyTo):
      {
        char *str = (char *)tag->ti_Data;

        setstring(data->ST_REPLYTO, str);
        if(IsStrEmpty(str) == FALSE)
          DoMethod(obj, METHOD(ShowRecipientObject), MUIV_WriteWindow_RcptType_ReplyTo);

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

      case ATTR(AttachContentType):
      {
        set(data->PO_CTYPE, MUIA_MimeTypePopup_MimeType, tag->ti_Data);

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
        set(data->CY_SIGNATURE, MUIA_SignatureChooser_Signature, tag->ti_Data);

        // make the superMethod call ignore those tags
        tag->ti_Tag = TAG_IGNORE;
      }
      break;

      case ATTR(Security):
      {
        setcycle(data->CY_SECURITY, tag->ti_Data);

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

      case ATTR(Modified):
      {
        data->mailModified = tag->ti_Data ? TRUE : FALSE;
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
                actObj == data->ST_REPLYTO ||
                actObj == data->ST_EXTHEADER || actObj == data->ST_DESC ||
                actObj == data->PO_CTYPE)
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
                actObj == data->ST_REPLYTO ||
                actObj == data->ST_EXTHEADER || actObj == data->ST_DESC ||
                actObj == data->PO_CTYPE)
        {
          DoMethod(actObj, MUIM_BetterString_DoAction, MUIV_BetterString_DoAction_Copy);
        }
        else if(actObj == data->LV_ATTACH || actObj == data->LV_ATTACH_TINY)
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
                actObj == data->ST_REPLYTO ||
                actObj == data->ST_EXTHEADER || actObj == data->ST_DESC ||
                actObj == data->PO_CTYPE)
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
                actObj == data->ST_REPLYTO ||
                actObj == data->ST_EXTHEADER || actObj == data->ST_DESC ||
                actObj == data->PO_CTYPE)
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
                actObj == data->ST_REPLYTO ||
                actObj == data->ST_EXTHEADER || actObj == data->ST_DESC ||
                actObj == data->PO_CTYPE)
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
                actObj == data->ST_REPLYTO ||
                actObj == data->ST_EXTHEADER || actObj == data->ST_DESC ||
                actObj == data->PO_CTYPE)
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
                actObj == data->ST_REPLYTO ||
                actObj == data->ST_EXTHEADER || actObj == data->ST_DESC ||
                actObj == data->PO_CTYPE)
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
                actObj == data->ST_REPLYTO ||
                actObj == data->ST_EXTHEADER || actObj == data->ST_DESC ||
                actObj == data->PO_CTYPE)
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

      DoMethod(obj, METHOD(AddAttachment), filename, NULL, FALSE);
    }
    else
    {
      int i;

      for(i=0; i < frc->numArgs; i++)
      {
        D(DBF_GUI, "chosen file: [%s] from drawer: [%s]", frc->argList[i], frc->drawer);

        AddPath(filename, frc->drawer, frc->argList[i], sizeof(filename));

        DoMethod(obj, METHOD(AddAttachment), filename, NULL, FALSE);
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

      DoMethod(obj, METHOD(AddAttachment), tf->Filename, "clipboard.text", TRUE);
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
// Adds ASCII version of user's public PGP key as attachment
DECLARE(AddPGPKey)
{
  GETDATA;
  struct UserIdentityNode *uin = data->wmData->identity;

  ENTER();

  if(uin->usePGP == TRUE)
  {
    char *pgpid = uin->pgpKeyID[0] != '\0' ? uin->pgpKeyID : uin->address;
    char options[SIZE_LARGE];
    const char *fname = "T:PubKey.asc";

    snprintf(options, sizeof(options), (G->PGPVersion == 5) ? "-x %s -o %s +force +batchmode=1" : "-kxa %s %s +f +bat", pgpid, fname);

    // on error returns > 0
    if(PGPCommand((G->PGPVersion == 5) ? "pgpk" : "pgp", options, 0) == 0)
    {
      LONG size = 0;

      if(ObtainFileInfo(fname, FI_SIZE, &size) == TRUE && size > 0)
      {
        DoMethod(obj, METHOD(AddAttachment), fname, NULL, TRUE);
        set(data->PO_CTYPE, MUIA_MimeTypePopup_MimeType, "application/pgp-keys");
      }
      else
        ER_NewError(tr(MSG_ER_ErrorAppendKey), pgpid);
    }
    else
      ER_NewError(tr(MSG_ER_ErrorAppendKey), pgpid);
  }
  else
    DisplayBeep(NULL);

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

  xset(obj, MUIA_Window_Title, data->windowTitle,
            MUIA_Window_ScreenTitle, CreateScreenTitle(data->screenTitle, sizeof(data->screenTitle), data->windowTitle));

  // the mail is modified
  data->mailModified = TRUE;

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
      if((command = dstralloc(SIZE_DEFAULT)) != NULL)
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
                dstrcat(&command, "%");
              break;

              case 'a':
              {
                D(DBF_UTIL, "insert archive name '%s'", arcpath);
                if(strchr(arcpath, ' ') != NULL)
                {
                  // surround the file name by quotes if it contains spaces
                  dstrcat(&command, "\"");
                  dstrcat(&command, arcpath);
                  // remember to add the closing quotes
                  mustCloseQuote = TRUE;
                }
                else
                  dstrcat(&command, arcpath);
              }
              break;

              case 'l':
              {
                D(DBF_UTIL, "insert filename '%s'", tf->Filename);
                if(strchr(tf->Filename, ' ') != NULL)
                {
                  // surround the file name by quotes if it contains spaces
                  dstrcat(&command, "\"");
                  dstrcat(&command, tf->Filename);
                  // remember to add the closing quotes
                  mustCloseQuote = TRUE;
                }
                else
                  dstrcat(&command, tf->Filename);
              }
              break;

              case 'f':
              {
                int i;

                for(i=0; i < frc->numArgs; i++)
                {
                  char filename[SIZE_PATHFILE];

                  snprintf(filename, sizeof(filename), "\"%s\" ", frc->argList[i]);
                  dstrcat(&command, filename);
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
              dstrcat(&command, "\" ");
              mustCloseQuote = FALSE;
            }
            else
            {
              // no quotes to be closed, just add the space character
              dstrcat(&command, " ");
            }
          }
          else
          {
            char chr[2];

            chr[0] = *src;
            chr[1] = '\0';

            dstrcat(&command, chr);
          }
        }

        // if there are still quotes to be closed do it now
        if(mustCloseQuote == TRUE)
          dstrcat(&command, "\"");

        // now we make the request drawer the current one temporarly.
        if((filedir = Lock(frc->drawer, ACCESS_READ)) != 0)
        {
          BPTR olddir = CurrentDir(filedir);

          // now we do the archive generation right here.
          if(LaunchCommand(command, 0, (C->ShowPackerProgress == TRUE) ? OUT_CONSOLE_CLOSE : OUT_NIL) == RETURN_OK)
            result = TRUE;

          // make the old directory the new current one again
          CurrentDir(olddir);
          UnLock(filedir);
        }

        // free our private resources
        dstrfree(command);
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

          DoMethod(obj, METHOD(AddAttachment), filename, NULL, TRUE);
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
/// DECLARE(RemoveAttachment)
// Deletes a file from the attachment list and the belonging temporary file
DECLARE(RemoveAttachment)
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

    // the mail is modified
    data->mailModified = TRUE;
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(RenameAttachment)
// Renames a file from the attachment list
DECLARE(RenameAttachment)
{
  GETDATA;
  struct Attach *attach = NULL;

  ENTER();

  // first we get the active entry
  DoMethod(data->LV_ATTACH, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &attach);
  if(attach != NULL)
  {
    char newName[SIZE_FILE];

    strlcpy(newName, attach->Name, sizeof(newName));
    if(StringRequest(newName, sizeof(newName), tr(MSG_WR_RENAME_ATTACHMENT), tr(MSG_WR_ENTER_NEW_NAME), tr(MSG_Okay), NULL, tr(MSG_Cancel), FALSE, obj))
    {
      if(strcmp(attach->Name, newName) != 0)
      {
        strlcpy(attach->Name, newName, sizeof(attach->Name));

        DoMethod(data->LV_ATTACH, MUIM_NList_Redraw, MUIV_NList_Redraw_Active);

        // the mail is modified
        data->mailModified = TRUE;
      }
    }
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
    if(FileExists(attach->FilePath) == TRUE)
      RE_DisplayMIME(attach->FilePath, NULL, attach->ContentType, FALSE);
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
  DoMethod(_app(obj), MUIM_MultiSet, MUIA_Disabled, attach ? FALSE : TRUE, data->PO_CTYPE,
                                                                           data->ST_DESC,
                                                                           data->BT_REMOVE,
                                                                           data->BT_RENAME,
                                                                           data->BT_DISPLAY, NULL);

  if(attach != NULL)
  {
    nnset(data->PO_CTYPE, MUIA_MimeTypePopup_MimeType, attach->ContentType);
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
    strlcpy(attach->ContentType, (char *)xget(data->PO_CTYPE, MUIA_MimeTypePopup_MimeType), sizeof(attach->ContentType));
    GetMUIString(attach->Description, data->ST_DESC, sizeof(attach->Description));
    DoMethod(data->LV_ATTACH, MUIM_NList_Redraw, MUIV_NList_Redraw_Active);
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(SignatureChanged)
// Changes the current signature
DECLARE(SignatureChanged)
{
  GETDATA;
  struct TempFile *tfin;

  ENTER();

  if((tfin = OpenTempFile(NULL)) != NULL)
  {
    FILE *in;
    Object *editor = data->TE_EDIT;

    DoMethod(editor, MUIM_MailTextEdit_SaveToFile, tfin->Filename, NULL);
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

        // add the signature or in case the signature is NULL
        // WriteSignature() will return immediatly
        WriteSignature(tfout->FP, (struct SignatureNode *)xget(data->CY_SIGNATURE, MUIA_SignatureChooser_Signature), TRUE);

        // now our out file is finished, so we can
        // put everything in our text editor.
        fclose(tfout->FP);
        tfout->FP = NULL;

        // free the buffer
        free(buf);

        // put everything in the editor
        flags = MUIF_NONE;
        if(data->useTextStyles == TRUE)
          setFlag(flags, MUIF_MailTextEdit_LoadFromFile_UseStyles);
        if(data->useTextColors == TRUE)
          setFlag(flags, MUIF_MailTextEdit_LoadFromFile_UseColors);
        if(xget(editor, MUIA_TextEditor_HasChanged))
          setFlag(flags, MUIF_MailTextEdit_LoadFromFile_SetChanged);

        DoMethod(editor, MUIM_MailTextEdit_LoadFromFile, tfout->Filename, NULL, flags);

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

        dstrfree(parsedText);
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
  {
    // don't trigger notifications as this will change the active object
    nnset(data->RG_PAGE, MUIA_Group_ActivePage, 0);
  }

  if((frc = ReqFile(ASL_ATTACH, obj, tr(MSG_WR_SaveTextAs), REQF_SAVEMODE, C->AttachDir, "")))
  {
    char filename[SIZE_PATHFILE];

    AddPath(filename, frc->drawer, frc->file, sizeof(filename));

    if(FileExists(filename) == FALSE ||
       MUI_Request(_app(obj), obj, MUIF_NONE, tr(MSG_MA_ConfirmReq), tr(MSG_YesNoReq), tr(MSG_FILE_OVERWRITE), frc->file) != 0)
    {
      if(DoMethod(data->TE_EDIT, MUIM_MailTextEdit_SaveToFile, filename, data->wmData->codeset) == FALSE)
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
      attach.IsTemp = msg->istemp;
      strlcpy(attach.FilePath, msg->filename, sizeof(attach.FilePath));
      strlcpy(attach.Name, msg->name ? msg->name : (char *)FilePart(msg->filename), sizeof(attach.Name));
      strlcpy(attach.ContentType, ctype, sizeof(attach.ContentType));
      nnset(data->PO_CTYPE, MUIA_MimeTypePopup_MimeType, attach.ContentType);
      nnset(data->ST_DESC, MUIA_String_Contents, attach.Description);

      // put the attachment into our attachment MUI list and set
      // it active.
      DoMethod(data->LV_ATTACH, MUIM_NList_InsertSingle, &attach, MUIV_NList_Insert_Bottom);
      set(data->LV_ATTACH, MUIA_NList_Active, MUIV_NList_Active_Bottom);
      set(data->GR_ATTACH_TINY, MUIA_ShowMe, TRUE);

      // the mail is modified
      data->mailModified = TRUE;
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
DECLARE(AddSignature)
{
  GETDATA;
  struct SignatureNode *sn;
  struct SignatureNode *signature = NULL;

  ENTER();

  // now we iterate through the signatureList
  // for picking the signature from the currently
  // selected identity
  IterateList(&C->signatureList, struct SignatureNode *, sn)
  {
    if(sn == data->wmData->identity->signature)
    {
      signature = data->wmData->identity->signature;
      break;
    }
  }

  if(signature != NULL)
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

      WriteSignature(fh_mail, signature, TRUE);
      fclose(fh_mail);
    }
  }

  // lets set the signature cycle gadget
  // accordingly to the set signature
  nnset(data->CY_SIGNATURE, MUIA_SignatureChooser_Signature, signature);

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
    case MUIV_WriteWindow_RcptType_From:
    {
      struct UserIdentityNode *uin;

      // try to match the identity by search through our user identities
      if((uin = FindUserIdentityByAddress(&C->userIdentityList, msg->recipient)) != NULL)
        set(data->CY_FROM, MUIA_IdentityChooser_Identity, uin);
      else
        DisplayBeep(NULL);
    }
    break;

    case MUIV_WriteWindow_RcptType_FromOverride:
      DoMethod(data->ST_FROM_OVERRIDE, MUIM_RecipientString_AddRecipient, msg->recipient);
    break;

    case MUIV_WriteWindow_RcptType_To:
      DoMethod(data->ST_TO, MUIM_RecipientString_AddRecipient, msg->recipient);
    break;

    case MUIV_WriteWindow_RcptType_CC:
      DoMethod(data->ST_CC, MUIM_RecipientString_AddRecipient, msg->recipient);
      DoMethod(obj, METHOD(ShowRecipientObject), MUIV_WriteWindow_RcptType_CC);
    break;

    case MUIV_WriteWindow_RcptType_BCC:
      DoMethod(data->ST_BCC, MUIM_RecipientString_AddRecipient, msg->recipient);
      DoMethod(obj, METHOD(ShowRecipientObject), MUIV_WriteWindow_RcptType_BCC);
    break;

    case MUIV_WriteWindow_RcptType_ReplyTo:
      DoMethod(data->ST_REPLYTO, MUIM_RecipientString_AddRecipient, msg->recipient);
      DoMethod(obj, METHOD(ShowRecipientObject), MUIV_WriteWindow_RcptType_ReplyTo);
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
    case MUIV_WriteWindow_RcptType_From:
    {
      struct UserIdentityNode *uin;

      // we try to match the addresses stored in **addr with the address
      // of our user identities. But as there can be only ONE address
      // we break out as soon as we have found one and beep otherwise
      do
      {
        // try to match the identity by search through our user identities
        if((uin = FindUserIdentityByAddress(&C->userIdentityList, *(msg->addr))) != NULL)
        {
          set(data->CY_FROM, MUIA_IdentityChooser_Identity, uin);
          break;
        }

        ++msg->addr;
      }
      while(*(msg->addr) != NULL);

      if(uin == NULL)
        DisplayBeep(NULL);
    }
    break;

    case MUIV_WriteWindow_RcptType_FromOverride:
      str = data->ST_FROM_OVERRIDE;
    break;

    case MUIV_WriteWindow_RcptType_To:
      str = data->ST_TO;
    break;

    case MUIV_WriteWindow_RcptType_CC:
      str = data->ST_CC;
      DoMethod(obj, METHOD(ShowRecipientObject), MUIV_WriteWindow_RcptType_CC);
    break;

    case MUIV_WriteWindow_RcptType_BCC:
      str = data->ST_BCC;
      DoMethod(obj, METHOD(ShowRecipientObject), MUIV_WriteWindow_RcptType_BCC);
    break;

    case MUIV_WriteWindow_RcptType_ReplyTo:
      str = data->ST_REPLYTO;
      DoMethod(obj, METHOD(ShowRecipientObject), MUIV_WriteWindow_RcptType_ReplyTo);
    break;
  }

  if(str != NULL)
  {
    if(msg->add == FALSE)
      setstring(str, "");

    do
    {
      DoMethod(str, MUIM_RecipientString_AddRecipient, *(msg->addr));
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
    struct codeset *dstCodeset;

    // stop any pending file notification.
    if(wmData->fileNotifyActive == TRUE)
    {
      EndNotify(wmData->notifyRequest);
      wmData->fileNotifyActive = FALSE;
    }

    // Workaround for a MUI bug
    if(xget(obj, MUIA_Window_Open) == TRUE)
    {
      // don't trigger notifications as this will change the active object
      nnset(data->RG_PAGE, MUIA_Group_ActivePage, 0);
    }

    // check if we should for a specific editor codeset
    if(C->DefaultEditorCodeset[0] != '\0')
    {
      dstCodeset = CodesetsFind(C->DefaultEditorCodeset,
                                CSA_CodesetList, G->codesetsList,
                                CSA_FallbackToDefault, FALSE);
    }
    else
      dstCodeset = data->wmData->codeset;

    // save the mail text in the currently selected codeset
    DoMethod(data->TE_EDIT, MUIM_MailTextEdit_SaveToFile, data->wmData->filename, dstCodeset);

    // remember the modification date of the file
    if(ObtainFileInfo(data->wmData->filename, FI_DATE, &data->wmData->lastFileChangeTime) == FALSE)
    {
      // use the current time in case ObtainFileInfo() failed
      DateStamp(&data->wmData->lastFileChangeTime);
    }

    snprintf(buffer, sizeof(buffer), "%s \"%s\"", C->Editor, GetRealPath(wmData->filename));
    LaunchCommand(buffer, LAUNCHF_ASYNC, OUT_NIL);

    // (re)start the file notification on the temporary write window
    // content file
    if(StartNotify(wmData->notifyRequest) != 0)
    {
      D(DBF_UTIL, "started notification request for file: '%s' of write window %ld", wmData->filename, data->windowNumber);
      wmData->fileNotifyActive = TRUE;
    }
    else
      W(DBF_UTIL, "file notification [%s] of write window %ld failed!", wmData->filename, data->windowNumber);
  }
  else
    DisplayBeep(NULL);

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

    DoMethod(data->TE_EDIT, MUIM_TextEditor_InsertText, text, MUIV_TextEditor_InsertText_Cursor);

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
    if((data->WI_SEARCH = SearchTextWindowObject, End))
    {
      // perform the search operation
      DoMethod(data->WI_SEARCH, MUIM_SearchTextWindow_Open, data->TE_EDIT);
    }
  }
  else
  {
    if(hasSearchAgainFlag(msg->flags))
      DoMethod(data->WI_SEARCH, MUIM_SearchTextWindow_Next);
    else
      DoMethod(data->WI_SEARCH, MUIM_SearchTextWindow_Open, data->TE_EDIT);
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

  DoMethod(data->TE_EDIT, MUIM_TextEditor_InsertText, msg->text, MUIV_TextEditor_InsertText_Cursor);

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
    setFlag(flags, MUIF_MailTextEdit_LoadFromFile_SetChanged);
  if(data->useTextStyles == TRUE)
    setFlag(flags, MUIF_MailTextEdit_LoadFromFile_UseStyles);
  if(data->useTextColors == TRUE)
    setFlag(flags, MUIF_MailTextEdit_LoadFromFile_UseColors);

  result = DoMethod(data->TE_EDIT, MUIM_MailTextEdit_LoadFromFile, data->wmData->filename, NULL, flags);

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
    setFlag(flags, MUIF_MailTextEdit_LoadFromFile_SetChanged);
  if(data->useTextStyles == TRUE)
    setFlag(flags, MUIF_MailTextEdit_LoadFromFile_UseStyles);
  if(data->useTextColors == TRUE)
    setFlag(flags, MUIF_MailTextEdit_LoadFromFile_UseColors);

  result = DoMethod(data->TE_EDIT, MUIM_MailTextEdit_LoadFromFile, msg->filename, NULL, flags);

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
      // use the CodesetChanged method to signal that the write
      // window should select the codeset of the letterPart of the
      // old mail
      DoMethod(obj, METHOD(CodesetChanged), part->CParCSet);
    }

    if(part->Nr != msg->rmData->letterPartNum &&
       stricmp(part->ContentType, "application/pgp-signature"))
    {
      struct Attach attach;
      struct BusyNode *busy;

      memset(&attach, 0, sizeof(struct Attach));

      busy = BusyBegin(BUSY_TEXT);
      BusyText(busy, tr(MSG_BusyDecSaving), "");

      RE_DecodePart(part);

      attach.Size = part->Size;
      attach.IsTemp = TRUE;

      if(part->Name)
        strlcpy(attach.Name, part->Name, sizeof(attach.Name));

      strlcpy(attach.FilePath, part->Filename, sizeof(attach.FilePath));
      *part->Filename = '\0';
      strlcpy(attach.ContentType, part->ContentType, sizeof(attach.ContentType));
      strlcpy(attach.Description, part->Description, sizeof(attach.Description));

      DoMethod(data->LV_ATTACH, MUIM_NList_InsertSingle, &attach, MUIV_NList_Insert_Bottom);

      BusyEnd(busy);
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
  struct Compose comp;
  struct MailList *newMailList = NULL;
  struct MailList *refMailList = NULL;
  char *addr;
  int numAttachments = 0;
  struct Folder *outfolder;
  BOOL winOpen = xget(obj, MUIA_Window_Open);
  struct WriteMailData *wmData = data->wmData;
  enum WriteMode mode = msg->mode;
  struct FolderNode *fnode;
  struct Folder *mlFolder = NULL;
  ULONG success = FALSE;

  ENTER();

  // clear some variables we fill up later on
  memset(&comp, 0, sizeof(struct Compose));

  // first we check all input values and fill up
  // the struct Compose structure

  // retrieve the From: address override but only
  // if the user has enabled the Rcpt gadget
  if(data->fromOverrideRcptHidden == FALSE)
    comp.FromOverride = (char *)xget(data->ST_FROM_OVERRIDE, MUIA_String_Contents);

  // get the contents of the TO: String gadget and check if it is valid
  addr = (char *)DoMethod(data->ST_TO, MUIM_RecipientString_Resolve, MUIF_RecipientString_Resolve_NoValid);
  if(mode != WRITE_DRAFT && addr == NULL)
  {
    ER_NewError(tr(MSG_ER_AliasNotFound), (STRPTR)xget(data->ST_TO, MUIA_String_Contents));

    if(winOpen == TRUE)
    {
      // don't trigger notifications as this will change the active object
      nnset(data->RG_PAGE, MUIA_Group_ActivePage, 0);
    }

    set(obj, MUIA_Window_ActiveObject, data->ST_TO);

    goto out;
  }
  else if(mode != WRITE_DRAFT && IsStrEmpty(addr) && wmData->quietMode == FALSE)
  {
    // set the TO Field active and go back
    if(winOpen == TRUE)
    {
      // don't trigger notifications as this will change the active object
      nnset(data->RG_PAGE, MUIA_Group_ActivePage, 0);
    }

    set(obj, MUIA_Window_ActiveObject, data->ST_TO);

    if(MUI_Request(_app(obj), obj, MUIF_NONE, NULL, tr(MSG_WR_NoRcptReqGad), tr(MSG_WR_ErrorNoRcpt)) != 0)
    {
      mode = WRITE_HOLD;
    }
    else
      goto out;
  }
  else if(IsStrEmpty(addr) == FALSE)
    comp.MailTo = addr; // To: address

  // get the content of the Subject: String gadget and check if it is empty or not.
  comp.Subject = (char *)xget(data->ST_SUBJECT, MUIA_String_Contents);
  if(wmData->mode != NMM_REDIRECT && mode != WRITE_DRAFT && wmData->quietMode == FALSE && C->WarnSubject == TRUE &&
     IsStrEmpty(comp.Subject))
  {
    char subject[SIZE_SUBJECT];

    if(winOpen == TRUE)
    {
      // don't trigger notifications as this will change the active object
      nnset(data->RG_PAGE, MUIA_Group_ActivePage, 0);
    }

    set(obj, MUIA_Window_ActiveObject, data->ST_SUBJECT);

    subject[0] = '\0';
    if(StringRequest(subject, sizeof(subject), NULL, tr(MSG_WR_NOSUBJECTREQ), tr(MSG_Okay), NULL, tr(MSG_Cancel), FALSE, obj) <= 0)
    {
      goto out;
    }
    else
    {
      // copy the entered subject back to string object
      // this time we don't care whether a subject was entered or not
      set(data->ST_SUBJECT, MUIA_String_Contents, subject);
      comp.Subject = (char *)xget(data->ST_SUBJECT, MUIA_String_Contents);
    }
  }

  // then we check the CC string gadget
  addr = (char *)DoMethod(data->ST_CC, MUIM_RecipientString_Resolve, MUIF_RecipientString_Resolve_NoValid);
  if(mode != WRITE_DRAFT && addr == NULL)
  {
    ER_NewError(tr(MSG_ER_AliasNotFound), (STRPTR)xget(data->ST_CC, MUIA_String_Contents));

    if(winOpen == TRUE)
    {
      // don't trigger notifications as this will change the active object
      nnset(data->RG_PAGE, MUIA_Group_ActivePage, 0);
    }

    set(obj, MUIA_Window_ActiveObject, data->ST_CC);

    goto out;
  }
  else if(IsStrEmpty(addr) == FALSE)
    comp.MailCC = addr;

  // then we check the BCC string gadget
  addr = (char *)DoMethod(data->ST_BCC, MUIM_RecipientString_Resolve, MUIF_RecipientString_Resolve_NoValid);
  if(mode != WRITE_DRAFT && addr == NULL)
  {
    ER_NewError(tr(MSG_ER_AliasNotFound), (STRPTR)xget(data->ST_BCC, MUIA_String_Contents));

    if(winOpen == TRUE)
    {
      // don't trigger notifications as this will change the active object
      nnset(data->RG_PAGE, MUIA_Group_ActivePage, 0);
    }

    set(obj, MUIA_Window_ActiveObject, data->ST_BCC);

    goto out;
  }
  else if(IsStrEmpty(addr) == FALSE)
    comp.MailBCC = addr;

  // from here on a mail redirect window/operation doesn't need to take care
  // of reply-to, attachments checking, etc.
  if(wmData->mode != NMM_REDIRECT)
  {
    // then we check the ReplyTo string gadget
    addr = (char *)DoMethod(data->ST_REPLYTO, MUIM_RecipientString_Resolve, MUIF_RecipientString_Resolve_NoValid);
    if(mode != WRITE_DRAFT && addr == NULL)
    {
      ER_NewError(tr(MSG_ER_AliasNotFound), (STRPTR)xget(data->ST_REPLYTO, MUIA_String_Contents));

      if(winOpen == TRUE)
      {
        // don't trigger notifications as this will change the active object
        nnset(data->RG_PAGE, MUIA_Group_ActivePage, 0);
      }

      set(obj, MUIA_Window_ActiveObject, data->ST_REPLYTO);

      goto out;
    }
    else if(IsStrEmpty(addr) == FALSE)
      comp.ReplyTo = addr;

    // now we search all To:,CC: and BCC: addresses and try to match them
    // against all mailing lists
    LockFolderListShared(G->folders);

    // walk through all folders and check if the
    // mailing list support matches
    ForEachFolderNode(G->folders, fnode)
    {
      struct Folder *curFolder = fnode->folder;

      if(curFolder != NULL && curFolder->MLSupport == TRUE &&
         curFolder->MLPattern[0] != '\0')
      {
        if((comp.MailTo != NULL && MatchNoCase(comp.MailTo, curFolder->MLPattern) == TRUE) ||
           (comp.MailCC != NULL && MatchNoCase(comp.MailCC, curFolder->MLPattern) == TRUE) ||
           (comp.MailBCC != NULL && MatchNoCase(comp.MailBCC, curFolder->MLPattern) == TRUE))
        {
          mlFolder = curFolder;
          break;
        }
      }
    }

    // if we found that a configued mailing list matches
    // we go and set Mail-Reply-To: and Mail-Followup-To:
    if(mlFolder != NULL && wmData->identity != NULL)
    {
      char address[SIZE_ADDRESS];
      comp.MailReplyTo = strdup(BuildAddress(address, sizeof(address), wmData->identity->address, wmData->identity->realname));
      comp.MailFollowupTo = strdup(mlFolder->MLAddress);
    }

    // unlock the folder list again
    UnlockFolderList(G->folders);

    // get the extra headers a user might have put into the mail
    comp.ExtHeader = (char *)xget(data->ST_EXTHEADER, MUIA_String_Contents);

    // In-Reply-To / References
    comp.inReplyToMsgID = wmData->inReplyToMsgID;
    comp.references = wmData->references;

    comp.Importance = 1-GetMUICycle(data->CY_IMPORTANCE);
    comp.RequestMDN = GetMUICheck(data->CH_MDN);
    comp.Signature = (struct SignatureNode *)xget(data->CY_SIGNATURE, MUIA_SignatureChooser_Signature);
    comp.Security = GetMUICycle(data->CY_SECURITY);
    comp.SelSecurity = comp.Security;
    comp.codeset = wmData->codeset;

    if(comp.Security == SEC_DEFAULTS &&
       SetDefaultSecurity(&comp, obj) == FALSE)
    {
      goto out;
    }

    comp.DelSend = GetMUICheck(data->CH_DELSEND);
    comp.UserInfo = GetMUICheck(data->CH_ADDINFO);

    numAttachments = xget(data->LV_ATTACH, MUIA_NList_Entries);

    // we execute the POSTWRITE macro right before writing out
    // the message because the postwrite macro may want to modify the
    // text in the editor beforehand.
    MA_StartMacro(MACRO_POSTWRITE, data->windowNumberStr);

    // export the text of our texteditor to a file in the currently selected codeset
    DoMethod(data->TE_EDIT, MUIM_MailTextEdit_SaveToFile, wmData->filename, wmData->codeset);

    // build the whole mail part list including the attachments
    // but don't delete temporary files when saving a draft mail
    if((comp.FirstPart = BuildPartsList(wmData, mode != WRITE_DRAFT)) == NULL)
    {
      goto out;
    }
  }
  else if(mode != WRITE_DRAFT)
    MA_StartMacro(MACRO_POSTWRITE, data->windowNumberStr);

  comp.Mode = wmData->mode;
  comp.Identity = wmData->identity;
  outfolder = FO_GetFolderByType(mode == WRITE_HOLD || mode == WRITE_DRAFT ? FT_DRAFTS : FT_OUTGOING, NULL);

  ////////////////////////////////
  // now we have checked that all input data is actually present
  // and valid (e.g. GUI gadgets, etc.) so we check if this write window was
  // created with referencing any mails or if this is a new mail write window
  if((newMailList = CreateMailList()) != NULL)
  {
    struct MailNode *mnode;
    int cntRefMail = -1;

    // check if this write window has some associated
    // referenced mail
    if(wmData->refMailList != NULL)
      refMailList = CloneMailList(wmData->refMailList);
    else if((refMailList = CreateMailList()) != NULL)
      AddNewMailNode(refMailList, wmData->refMail);
    else
      goto out;

    // now iterate through the local refMailList
    ForEachMailNode(refMailList, mnode)
    {
      struct Mail *refMail = mnode->mail;
      struct Mail *newMail = NULL;
      char newMailFile[SIZE_PATHFILE];

      // count the number of reference
      // mails we processed already
      cntRefMail++;

      // forget the previous file name
      newMailFile[0] = '\0';

      // now we check how the new mail file should be named
      // or created. As we iterate through all refMailList
      // nodes we only generate a new file for the mail redirecting
      // mode. All other modes can't have multiple output files
      if(cntRefMail == 0 || wmData->mode == NMM_REDIRECT)
      {
        switch(wmData->mode)
        {
          case NMM_EDIT:
          {
            if(refMail != NULL && MailExists(refMail, outfolder) == TRUE)
            {
              GetMailFile(newMailFile, sizeof(newMailFile), outfolder, refMail);
              break;
            }
          }
          // continue

          case NMM_EDITASNEW:
            wmData->mode = NMM_NEW;
          // continue

          default:
          {
            if(isDraftsFolder(outfolder) && wmData->draftMail != NULL && MailExists(wmData->draftMail, outfolder) == TRUE)
              GetMailFile(newMailFile, sizeof(newMailFile), outfolder, wmData->draftMail);
            else
              MA_NewMailFile(outfolder, newMailFile, sizeof(newMailFile));
          }
          break;
        }
      }

      if(newMailFile[0] != '\0')
      {
        // now open the new mail file for write operations
        if((comp.FH = fopen(newMailFile, "w")) != NULL)
        {
          struct ExtendedMail *email = NULL;

          // change file buffer settings
          setvbuf(comp.FH, NULL, _IOFBF, SIZE_FILEBUF);

          // Write out the message to our file and check that everything worked out fine.
          // Set a busy mouse pointer during this operation, since encoding large attachments
          // might take quite a long time.
          set(obj, MUIA_Window_Sleep, TRUE);
          comp.refMail = refMail;
          success = WriteOutMessage(&comp);
          set(obj, MUIA_Window_Sleep, FALSE);

          // close the file handle immediately
          fclose(comp.FH);
          comp.FH = NULL;

          // if the WriteOut operation didn't work
          // as expected stop here
          if(success == FALSE)
          {
            DeleteFile(newMailFile);
            goto out;
          }

          // stop any pending file notification.
          if(wmData->fileNotifyActive == TRUE)
          {
            EndNotify(wmData->notifyRequest);
            wmData->fileNotifyActive = FALSE;
          }

          if((email = MA_ExamineMail(outfolder, FilePart(newMailFile), C->EmailCache > 0 ? TRUE : FALSE)) != NULL)
          {
            if((newMail = CloneMail(&email->Mail)) != NULL)
            {
              struct Mail *replacedMail = NULL;

              if(mode == WRITE_DRAFT)
              {
                replacedMail = ReplaceMailInFolder(FilePart(newMailFile), newMail, outfolder);
              }
              else
              {
                AddMailToFolder(newMail, outfolder);
                replacedMail = NULL;
              }

              // Now we have to check whether we have to add the To & CC addresses
              // to the emailCache
              if(C->EmailCache > 0)
              {
                DoMethod(_app(obj), MUIM_YAMApplication_AddToEmailCache, &newMail->To);

                // if this mail has more than one recipient we have to add the others too
                if(isMultiRCPTMail(newMail))
                {
                  int j;

                  for(j = 0; j < email->NumSTo; j++)
                    DoMethod(_app(obj), MUIM_YAMApplication_AddToEmailCache, &email->STo[j]);

                  for(j = 0; j < email->NumCC; j++)
                    DoMethod(_app(obj), MUIM_YAMApplication_AddToEmailCache, &email->CC[j]);

                  for(j = 0; j < email->NumBCC; j++)
                    DoMethod(_app(obj), MUIM_YAMApplication_AddToEmailCache, &email->BCC[j]);
                }
              }

              if(GetCurrentFolder() == outfolder)
              {
                if(replacedMail != NULL)
                {
                  // find the old mail, replace it and resort the list
                  LONG pos = MUIV_NList_GetPos_Start;

                  DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_NList_GetPos, replacedMail, &pos);

                  set(G->MA->GUI.PG_MAILLIST, MUIA_NList_Quiet, TRUE);
                  DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_NList_ReplaceSingle, newMail, pos, NOWRAP, ALIGN_LEFT);
                  DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_NList_Sort);
                  set(G->MA->GUI.PG_MAILLIST, MUIA_NList_Quiet, FALSE);

                  // free the replaced mail
                  FreeMail(replacedMail);
                }
                else
                {
                  DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_NList_InsertSingle, newMail, MUIV_NList_Insert_Sorted);
                }
              }

              MA_UpdateMailFile(newMail);

              // if this write operation was an edit mode
              // we have to check all existing readmail objects for
              // references and update them accordingly.
              if(wmData->mode == NMM_EDIT && refMail != NULL)
              {
                struct ReadMailData *rmData;

                // now we search through our existing readMailData
                // objects and see some of them are pointing to the old mail
                // and if so we signal them to display the new revised mail instead
                IterateList(&G->readMailDataList, struct ReadMailData *, rmData)
                {
                  if(rmData->mail == refMail)
                  {
                    if(rmData->readWindow != NULL)
                      DoMethod(rmData->readWindow, MUIM_ReadWindow_ReadMail, newMail);
                    else if(rmData->readMailGroup != NULL)
                      DoMethod(rmData->readMailGroup, MUIM_ReadMailGroup_ReadMail, newMail);
                  }
                }

                // increase the mail's reference counter to prevent RemoveMailFromFolder() from
                // freeing the mail in its DeleteMailNode() call
                ReferenceMail(refMail);

                // remove the mail
                RemoveMailFromFolder(refMail, TRUE, TRUE);

                // decrease the reference counter again and free the mail
                DereferenceMail(refMail);

                refMail = newMail;
              }
              else if(wmData->mode == NMM_NEW && refMail != NULL)
                refMail = newMail;

              // replace the mail pointer in the reference mail list
              mnode->mail = refMail;

              if(wmData->draftMail != NULL || mode == WRITE_DRAFT)
              {
                if(mode == WRITE_SEND || mode == WRITE_QUEUE)
                {
                  // delete the mail from the drafts folder
                  MA_DeleteSingle(wmData->draftMail, DELF_AT_ONCE);
                }

                // remember the new mail as draft mail
                wmData->draftMail = newMail;
              }

              // add the new mail to our newMailList if
              // we have to sent it later
              AddNewMailNode(newMailList, newMail);
            }

            // cleanup the email structure
            MA_FreeEMailStruct(email);
          }
        }
        else
        {
          ER_NewError(tr(MSG_ER_CANNOT_CREATE_MAIL_FILE), newMailFile);
          goto out;
        }
      }

      // now we make sure the Disposition Notification is respected
      // and the status of the referenced mail updated accordingly.
      if(wmData->mode != NMM_NEW)
      {
        if(refMail != NULL && !isVirtualMail(refMail) && refMail->Folder != NULL &&
           !isOutgoingFolder(refMail->Folder) && !isDraftsFolder(refMail->Folder) && !isSentFolder(refMail->Folder))
        {
          // process MDN notifications
          if(hasStatusNew(refMail) || !hasStatusRead(refMail))
            RE_ProcessMDN(MDN_MODE_DISPLAY, refMail, FALSE, wmData->quietMode, obj);

          switch(wmData->mode)
          {
            case NMM_REPLY:
            {
              setStatusToReplied(refMail);
              DisplayStatistics(refMail->Folder, FALSE);
            }
            break;

            case NMM_FORWARD:
            case NMM_FORWARD_ATTACH:
            case NMM_FORWARD_INLINE:
            case NMM_REDIRECT:
            {
              // don't change the forwarded flag if we are just saving a draft mail
              if(mode != WRITE_DRAFT)
              {
                setStatusToForwarded(refMail);
                DisplayStatistics(refMail->Folder, FALSE);
              }
            }
            break;

            default:
              // nothing
            break;
          }
        }
      }

      // if requested we make sure we also
      // output a log entry about the write operation
      switch(wmData->mode)
      {
        case NMM_NEW:
        case NMM_EDITASNEW:
        {
          if(newMail != NULL)
            AppendToLogfile(LF_ALL, 10, tr(MSG_LOG_Creating), AddrName(newMail->To), newMail->Subject, numAttachments);
        }
        break;

        case NMM_REPLY:
        {
          if(refMail != NULL)
            AppendToLogfile(LF_ALL, 11, tr(MSG_LOG_Replying), AddrName(refMail->From), refMail->Subject);
          else
            AppendToLogfile(LF_ALL, 11, tr(MSG_LOG_Replying), "<unknown>", "<unknown>");
        }
        break;

        case NMM_FORWARD:
        case NMM_FORWARD_ATTACH:
        case NMM_FORWARD_INLINE:
        {
          if(newMail != NULL)
          {
            if(refMail != NULL)
              AppendToLogfile(LF_ALL, 12, tr(MSG_LOG_Forwarding), AddrName(refMail->From), refMail->Subject, AddrName(newMail->To));
            else
              AppendToLogfile(LF_ALL, 12, tr(MSG_LOG_Forwarding), "<unknown>", "<unknown>", AddrName(newMail->To));
          }
        }
        break;

        case NMM_REDIRECT:
        {
          if(newMail != NULL)
          {
            if(refMail != NULL)
              AppendToLogfile(LF_ALL, 13, tr(MSG_LOG_REDIRECT), AddrName(refMail->From), refMail->Subject, AddrName(newMail->To));
            else
              AppendToLogfile(LF_ALL, 13, tr(MSG_LOG_REDIRECT), "<unknown>", "<unknown>", AddrName(newMail->To));
          }
        }
        break;

        case NMM_EDIT:
        {
          if(newMail != NULL)
            AppendToLogfile(LF_ALL, 14, tr(MSG_LOG_Editing), AddrName(newMail->From), AddrName(newMail->To), newMail->Subject);
        }
        break;

        case NMM_SAVEDEC:
          // not used
        break;
      }
    }
  }

  if(mode == WRITE_DRAFT)
  {
    // the mail is no longer modified
    data->mailModified = FALSE;

    // we just saved the mail text, so it is no longer modified
    set(data->TE_EDIT, MUIA_TextEditor_HasChanged, FALSE);

    // forget any "auto save" action
    // if this method is called as an "auto save" action this flag will be restored right after the call
    data->autoSaved = FALSE;
  }
  else
  {
    // cleanup certain references if the window is to be closed
    wmData->refMail = NULL;
    wmData->draftMail = NULL;

    if(wmData->refMailList != NULL)
    {
      DeleteMailList(wmData->refMailList);
      wmData->refMailList = NULL;
    }

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
  }

  // now we make sure we immediately send out the mail.
  if(mode == WRITE_SEND && newMailList != NULL)
  {
    struct UserIdentityNode *uin = wmData->identity;

    if((uin->sentMailList = CloneMailList(newMailList)) != NULL)
    {
      BOOL mailSent = FALSE;

      // close the write window now
      set(obj, MUIA_Window_Open, FALSE);

      if(uin->smtpServer != NULL)
      {
        if(hasServerInUse(uin->smtpServer) == FALSE)
        {
          // mark the server as "in use"
          setFlag(uin->smtpServer->flags, MSF_IN_USE);

          mailSent = (DoAction(NULL, TA_SendMails, TT_SendMails_UserIdentity, uin,
                                                   TT_SendMails_Mode, SENDMAIL_ACTIVE_USER,
                                                   TAG_DONE) != NULL);
          if(mailSent == FALSE)
            clearFlag(uin->smtpServer->flags, MSF_IN_USE);
        }
        else
          W(DBF_MAIL, "SMTP server '%s' in use, couldn't sent out mail", uin->smtpServer->description);
      }

      if(mailSent == FALSE)
      {
        DeleteMailList(uin->sentMailList);
        uin->sentMailList = NULL;
      }
    }
  }

  // make sure to dispose the window data by calling
  // CleanupWriteMailData method as soon as this method is finished
  if(mode != WRITE_DRAFT)
    DoMethod(_app(obj), MUIM_Application_PushMethod, _app(obj), 2, MUIM_YAMApplication_CleanupWriteMailData, wmData);

  // update the statistics of the outgoing folder
  DisplayStatistics(outfolder, TRUE);

  // if we end up here then everything was successful
  success = TRUE;

out:
  if(newMailList != NULL)
    DeleteMailList(newMailList);

  if(refMailList != NULL)
    DeleteMailList(refMailList);

  // free the list of MIME parts but don't delete temporary
  // files when saving a draft mail
  FreePartsList(comp.FirstPart, mode != WRITE_DRAFT);

  // free the compose structure
  FreeCompose(&comp);

  RETURN(success);
  return success;
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
      DoMethod(data->TE_EDIT, MUIM_TextEditor_InsertText, text, MUIV_TextEditor_InsertText_Cursor);
      free(text);
    }
    else
      DoMethod(obj, METHOD(AddAttachment), msg->fileName, NULL, FALSE);
  }
  else
    DoMethod(obj, METHOD(AddAttachment), msg->fileName, NULL, FALSE);

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

  // Redirected mails are never saved automatically.
  // There is not much to loose as the window lets
  // the user enter new addresses only anyway.
  if(data->wmData->mode != NMM_REDIRECT)
  {
    // do the autosave only if something was modified
    if(data->mailModified == TRUE || xget(data->TE_EDIT, MUIA_TextEditor_HasChanged) == TRUE)
    {
      // prevent the autosave from happening when one of the match windows of
      // the recipientstring objects are open or otherwise they are intermediately
      // closed and resolved as soon as the autosave happens.
      if(xget(data->ST_TO, MUIA_RecipientString_MatchwindowOpen) == FALSE &&
         xget(data->ST_CC, MUIA_RecipientString_MatchwindowOpen) == FALSE &&
         xget(data->ST_BCC, MUIA_RecipientString_MatchwindowOpen) == FALSE &&
         xget(data->ST_REPLYTO, MUIA_RecipientString_MatchwindowOpen) == FALSE)
      {
        if(DoMethod(obj, METHOD(ComposeMail), WRITE_DRAFT) == TRUE)
        {
          // we must remember if the mail was automatically saved, since the editor object cannot
          // tell about changes anymore if they don't happen from now on.
          data->autoSaved = TRUE;

          D(DBF_MAIL, "saved mail text of write window #%d", data->windowNumber);
        }
      }
      else
        W(DBF_MAIL, "match window of recipientstring open, rejected autosave of write window #%d", data->windowNumber);
    }
    else
      D(DBF_MAIL, "no changes found in editor, no need to save an autosave file");
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(CancelAction)
// CancelAction - User clicked the close button
DECLARE(CancelAction)
{
  GETDATA;
  BOOL discard = TRUE;

  ENTER();

  if(data->wmData->quietMode == FALSE)
  {
    // ask the user what to do if the mail text was modified but not yet automatically saved
    if(data->mailModified == TRUE ||
       data->autoSaved == TRUE ||
       (data->wmData->mode != NMM_REDIRECT && xget(data->TE_EDIT, MUIA_TextEditor_HasChanged) == TRUE))
    {
      switch(MUI_Request(_app(obj), obj, MUIF_NONE, NULL, tr(MSG_WR_DiscardChangesGad), tr(MSG_WR_DiscardChanges)))
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
          DoMethod(obj, METHOD(ComposeMail), WRITE_QUEUE);
          discard = FALSE;
        }
        break;

        case 2:
        {
          // discard
          // remove a previously saved draft mail from the drafts folder
          if(data->wmData->draftMail != NULL)
            MA_DeleteSingle(data->wmData->draftMail, DELF_AT_ONCE);
        }
        break;
      }
    }
  }

  // check if we have to close/discard the window
  // However, please note that because we do kill the window upon closing it
  // we have to use MUIM_Application_PushMethod instead of calling the
  // CleanupWriteMailData method directly
  if(discard == TRUE)
    DoMethod(_app(obj), MUIM_Application_PushMethod, _app(obj), 2, MUIM_YAMApplication_CleanupWriteMailData, data->wmData);

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
        if(MUI_Request(_app(obj), obj, MUIF_NONE, tr(MSG_FCHANGE_WARN_TITLE),
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
        DoMethod(obj, METHOD(ReloadText), TRUE);

        // remember this new date stamp
        memcpy(&data->wmData->lastFileChangeTime, &currentFileChangeTime, sizeof(data->wmData->lastFileChangeTime));
      }
    }
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(IdentityChanged)
//
DECLARE(IdentityChanged) // struct UserIdentityNode *uin;
{
  GETDATA;

  ENTER();

  // the user has changed the identity, thus we update certain things
  // in the write window according to the settings in the user identity.

  // first we update things in the write window GUI
  setstring(data->ST_FROM_OVERRIDE, msg->uin->address);
  set(data->ST_TO, MUIA_RecipientString_ActiveIdentity, msg->uin);
  xset(data->ST_CC, MUIA_String_Contents, msg->uin->mailCC,
                    MUIA_RecipientString_ActiveIdentity, msg->uin);
  xset(data->ST_BCC, MUIA_String_Contents, msg->uin->mailBCC,
                     MUIA_RecipientString_ActiveIdentity, msg->uin);

  // just go on in non redirection mode
  if(data->wmData->mode != NMM_REDIRECT)
  {
    set(data->CY_SIGNATURE, MUIA_SignatureChooser_Signature, msg->uin->signature);
    set(data->ST_REPLYTO, MUIA_String_Contents, msg->uin->mailReplyTo);
    set(data->ST_EXTHEADER, MUIA_String_Contents, msg->uin->extraHeaders);
    set(data->CH_DELSEND, MUIA_Selected, msg->uin->saveSentMail == FALSE);
    set(data->CH_ADDINFO, MUIA_Selected, msg->uin->addPersonalInfo);
    set(data->CH_MDN, MUIA_Selected, msg->uin->requestMDN);
    set(data->ST_REPLYTO, MUIA_RecipientString_ActiveIdentity, msg->uin);

    // check if we have to show the CC/BCC/ReplyTo string objects
    // in case we filled in sensible information
    if(msg->uin->mailCC[0] != '\0')
      DoMethod(obj, METHOD(ShowRecipientObject), MUIV_WriteWindow_RcptType_CC);

    if(msg->uin->mailBCC[0] != '\0')
      DoMethod(obj, METHOD(ShowRecipientObject), MUIV_WriteWindow_RcptType_BCC);

    if(msg->uin->mailReplyTo[0] != '\0')
      DoMethod(obj, METHOD(ShowRecipientObject), MUIV_WriteWindow_RcptType_ReplyTo);
  }

  // save a link to the userIdentity in the wmData
  data->wmData->identity = msg->uin;

  // the mail is modified
  data->mailModified = TRUE;

  RETURN(0);
  return 0;
}

///
/// DECLARE(HideRecipientObject)
//
DECLARE(HideRecipientObject) // enum RcptType rtype
{
  GETDATA;
  ENTER();

  if(DoMethod(data->GR_HEADER, MUIM_Group_InitChange))
  {
    switch(msg->rtype)
    {
      case MUIV_WriteWindow_RcptType_From:
      {
        // we only remove it if it is already shown
        if(data->fromRcptHidden == FALSE)
        {
          DoMethod(data->GR_HEADER, OM_REMMEMBER, data->LB_FROM);
          DoMethod(data->GR_HEADER, OM_REMMEMBER, data->CY_FROM);
          data->fromRcptHidden = TRUE;
        }
      }
      break;

      case MUIV_WriteWindow_RcptType_FromOverride:
      {
        // we only remove it if it is already shown
        if(data->fromOverrideRcptHidden == FALSE)
        {
          DoMethod(data->GR_HEADER, OM_REMMEMBER, data->LB_FROM_OVERRIDE);
          DoMethod(data->GR_HEADER, OM_REMMEMBER, data->GR_FROM_OVERRIDE);
          data->fromOverrideRcptHidden = TRUE;
        }
      }
      break;

      case MUIV_WriteWindow_RcptType_CC:
      {
        // we only remove it if it is already shown
        if(data->ccRcptHidden == FALSE)
        {
          DoMethod(data->GR_HEADER, OM_REMMEMBER, data->LB_CC);
          DoMethod(data->GR_HEADER, OM_REMMEMBER, data->GR_CC);
          data->ccRcptHidden = TRUE;

          // make sure to set the menuitem object state correctly
          nnset(data->MN_CC, MUIA_Menuitem_Checked, FALSE);
        }
      }
      break;

      case MUIV_WriteWindow_RcptType_BCC:
      {
        // we only remove it if it is already shown
        if(data->bccRcptHidden == FALSE)
        {
          DoMethod(data->GR_HEADER, OM_REMMEMBER, data->LB_BCC);
          DoMethod(data->GR_HEADER, OM_REMMEMBER, data->GR_BCC);
          data->bccRcptHidden = TRUE;

          // make sure to set the menuitem object state correctly
          nnset(data->MN_BCC, MUIA_Menuitem_Checked, FALSE);
        }
      }
      break;

      case MUIV_WriteWindow_RcptType_ReplyTo:
      {
        // we only remove it if it is already shown
        if(data->replyToRcptHidden == FALSE)
        {
          DoMethod(data->GR_HEADER, OM_REMMEMBER, data->LB_REPLYTO);
          DoMethod(data->GR_HEADER, OM_REMMEMBER, data->GR_REPLYTO);
          data->replyToRcptHidden = TRUE;

          // make sure to set the menuitem object state correctly
          nnset(data->MN_REPLYTO, MUIA_Menuitem_Checked, FALSE);
        }
      }
      break;

      case MUIV_WriteWindow_RcptType_To:
        // do nothing
      break;
    }

    DoMethod(data->GR_HEADER, MUIM_Group_ExitChange);
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(ShowRecipientObject)
//
DECLARE(ShowRecipientObject) // enum RcptType rtype
{
  GETDATA;

  ENTER();

  if(DoMethod(data->GR_HEADER, MUIM_Group_InitChange))
  {
    struct {
      ULONG MethodID;
      Object *objs[16];
    } sortMsg;
    int objcnt;

    switch(msg->rtype)
    {
      case MUIV_WriteWindow_RcptType_From:
      {
        // we only show it if it is already hidden
        if(data->fromRcptHidden == TRUE)
        {
          DoMethod(data->GR_HEADER, OM_ADDMEMBER, data->LB_FROM);
          DoMethod(data->GR_HEADER, OM_ADDMEMBER, data->CY_FROM);

          data->fromRcptHidden = FALSE;
        }
      }
      break;

      case MUIV_WriteWindow_RcptType_FromOverride:
      {
        // we only show it if it is already hidden
        if(data->fromOverrideRcptHidden == TRUE)
        {
          DoMethod(data->GR_HEADER, OM_ADDMEMBER, data->LB_FROM_OVERRIDE);
          DoMethod(data->GR_HEADER, OM_ADDMEMBER, data->GR_FROM_OVERRIDE);

          data->fromOverrideRcptHidden = FALSE;
        }
      }
      break;

      case MUIV_WriteWindow_RcptType_CC:
      {
        // we only show it if it is already hidden
        if(data->ccRcptHidden == TRUE)
        {
          DoMethod(data->GR_HEADER, OM_ADDMEMBER, data->LB_CC);
          DoMethod(data->GR_HEADER, OM_ADDMEMBER, data->GR_CC);

          data->ccRcptHidden = FALSE;

          // make sure to set the menuitem object state correctly
          nnset(data->MN_CC, MUIA_Menuitem_Checked, TRUE);
        }
      }
      break;

      case MUIV_WriteWindow_RcptType_BCC:
      {
        // we only show it if it is already hidden
        if(data->bccRcptHidden == TRUE)
        {
          DoMethod(data->GR_HEADER, OM_ADDMEMBER, data->LB_BCC);
          DoMethod(data->GR_HEADER, OM_ADDMEMBER, data->GR_BCC);

          data->bccRcptHidden = FALSE;

          // make sure to set the menuitem object state correctly
          nnset(data->MN_BCC, MUIA_Menuitem_Checked, TRUE);
        }
      }
      break;

      case MUIV_WriteWindow_RcptType_ReplyTo:
      {
        // we only show it if it is already hidden
        if(data->replyToRcptHidden == TRUE)
        {
          DoMethod(data->GR_HEADER, OM_ADDMEMBER, data->LB_REPLYTO);
          DoMethod(data->GR_HEADER, OM_ADDMEMBER, data->GR_REPLYTO);

          data->replyToRcptHidden = FALSE;

          // make sure to set the menuitem object state correctly
          nnset(data->MN_REPLYTO, MUIA_Menuitem_Checked, TRUE);
        }
      }
      break;

      case MUIV_WriteWindow_RcptType_To:
        // do nothing
      break;
    }

    // set up the parameters for MUIM_Group_Sort as MUIM_Group_MoveMember
    // seems to be broken in MUI 3.8
    sortMsg.MethodID = MUIM_Group_Sort;

    objcnt = 0;
    if(data->fromRcptHidden == FALSE)
    {
      sortMsg.objs[objcnt] = data->LB_FROM; objcnt++;
      sortMsg.objs[objcnt] = data->CY_FROM; objcnt++;
    }

    if(data->fromOverrideRcptHidden == FALSE)
    {
      sortMsg.objs[objcnt] = data->LB_FROM_OVERRIDE; objcnt++;
      sortMsg.objs[objcnt] = data->GR_FROM_OVERRIDE; objcnt++;
    }

    sortMsg.objs[objcnt] = data->LB_TO; objcnt++;
    sortMsg.objs[objcnt] = data->GR_TO; objcnt++;

    if(data->ccRcptHidden == FALSE)
    {
      sortMsg.objs[objcnt] = data->LB_CC; objcnt++;
      sortMsg.objs[objcnt] = data->GR_CC; objcnt++;
    }

    if(data->bccRcptHidden == FALSE)
    {
      sortMsg.objs[objcnt] = data->LB_BCC; objcnt++;
      sortMsg.objs[objcnt] = data->GR_BCC; objcnt++;
    }

    // all the other objects are just part of a normal write
    // window and not a redirecting window
    if(data->wmData->mode != NMM_REDIRECT)
    {
      if(data->replyToRcptHidden == FALSE)
      {
        sortMsg.objs[objcnt] = data->LB_REPLYTO; objcnt++;
        sortMsg.objs[objcnt] = data->GR_REPLYTO; objcnt++;
      }
      sortMsg.objs[objcnt] = data->LB_SUBJECT; objcnt++;
      sortMsg.objs[objcnt] = data->ST_SUBJECT; objcnt++;
    }

    // terminate the array
    sortMsg.objs[objcnt] = NULL;

    // sort the objects
    DoMethodA(data->GR_HEADER, (Msg)&sortMsg);

    // make sure to show only one "From:" label
    if(data->fromOverrideRcptHidden == FALSE)
      set(data->LB_FROM_OVERRIDE, MUIA_Group_ActivePage, (NumberOfUserIdentities(&C->userIdentityList) == 1) ? 1 : 0);

    DoMethod(data->GR_HEADER, MUIM_Group_ExitChange);
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(MenuToggleRecipientObject)
//
DECLARE(MenuToggleRecipientObject) // enum RcptType rtype
{
  GETDATA;
  BOOL show = TRUE;
  BOOL saveConfig = FALSE;
  ENTER();

  switch(msg->rtype)
  {
    case MUIV_WriteWindow_RcptType_From:
      show = (data->fromRcptHidden == TRUE);
    break;

    case MUIV_WriteWindow_RcptType_FromOverride:
      show = (data->fromOverrideRcptHidden == TRUE);
    break;

    case MUIV_WriteWindow_RcptType_CC:
    {
      show = (data->ccRcptHidden == TRUE);

      // if the user used the menuitem we set a new default
      // in the configuration
      if(C->ShowRcptFieldCC != show)
      {
        // save new value to global config
        C->ShowRcptFieldCC = show;

        // also save it to the temp CE struct in
        // case the config window exists (is open)
        if(G->ConfigWinObject != NULL && CE != NULL)
          CE->ShowRcptFieldCC = show;

        saveConfig = TRUE;
      }
    }
    break;

    case MUIV_WriteWindow_RcptType_BCC:
    {
      show = (data->bccRcptHidden == TRUE);

      // if the user used the menuitem we set a new default
      // in the configuration
      if(C->ShowRcptFieldBCC != show)
      {
        // save new value to global config
        C->ShowRcptFieldBCC = show;

        // also save it to the temp CE struct in
        // case the config window exists (is open)
        if(G->ConfigWinObject != NULL && CE != NULL)
          CE->ShowRcptFieldBCC = show;

        saveConfig = TRUE;
      }
    }
    break;

    case MUIV_WriteWindow_RcptType_ReplyTo:
    {
      show = (data->replyToRcptHidden == TRUE);

      // if the user used the menuitem we set a new default
      // in the configuration
      if(C->ShowRcptFieldReplyTo != show)
      {
        // save new value to global config
        C->ShowRcptFieldReplyTo = show;

        // also save it to the temp CE struct in
        // case the config window exists (is open)
        if(G->ConfigWinObject != NULL && CE != NULL)
          CE->ShowRcptFieldReplyTo = show;

        saveConfig = TRUE;
      }
    }
    break;

    case MUIV_WriteWindow_RcptType_To:
      // do nothing
    break;
  }

  // make sure to show/hide the recipient object
  if(show == TRUE)
    DoMethod(obj, METHOD(ShowRecipientObject), msg->rtype);
  else
    DoMethod(obj, METHOD(HideRecipientObject), msg->rtype);

  // then we save the new default state for the
  // manually hidden/shown recipient object
  if(saveConfig == TRUE)
    SaveConfig(C, G->CO_PrefsFile);

  RETURN(0);
  return 0;
}

///
/// DECLARE(UpdateSignatures)
// update the signature menu
DECLARE(UpdateSignatures)
{
  GETDATA;
  int i;
  struct SignatureNode *activeSig = data->wmData->identity->signature;
  struct SignatureNode *sn;
  int activeSigIndex = 0;
  Object *item;

  ENTER();

  // first remove all previous items and notifications
  DoMethod(data->CY_SIGNATURE,  MUIM_KillNotifyObj, MUIA_Cycle_Active, data->MI_SIGNATURE);
  for(i = 0; i < MAXSIG_MENU; i++)
  {
    if(data->MI_SIGNATURES[i] != NULL)
    {
      DoMethod(obj, MUIM_KillNotify, WMEN_SIGN0+i);
      DoMethod(data->MI_SIGNATURE, MUIM_Family_Remove, data->MI_SIGNATURES[i]);
      MUI_DisposeObject(data->MI_SIGNATURES[i]);
      data->MI_SIGNATURES[i] = NULL;
    }
  }

  // update the signature chooser
  DoMethod(data->CY_SIGNATURE, MUIM_SignatureChooser_UpdateSignatures);

  // add the "no signature" menu item
  item = MenuitemCheck(tr(MSG_CO_IDENTITY_NOSIGNATURE), "0", TRUE, FALSE, 0, 0xff & ~(1<<0), WMEN_SIGN0);
  if(item != NULL)
  {
    data->MI_SIGNATURES[0] = item;
    DoMethod(data->MI_SIGNATURE, MUIM_Family_AddTail, item);
    // set up the notifications
    DoMethod(item, MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime, data->CY_SIGNATURE, 3, MUIM_Set, MUIA_Cycle_Active, 0);
    DoMethod(data->CY_SIGNATURE, MUIM_Notify, MUIA_Cycle_Active, 0, data->MI_SIGNATURE, 4, MUIM_SetUData, WMEN_SIGN0, MUIA_Menuitem_Checked, TRUE);
  }

  // add the first 7 signatures from the config
  i = 1;
  IterateList(&C->signatureList, struct SignatureNode *, sn)
  {
    if(sn->active == TRUE)
    {
      if(i < MAXSIG_MENU)
      {
        // the first 3 signatures get a shortcut
        switch(i)
        {
          case 1:
            item = MenuitemCheck(sn->description, "7", TRUE, FALSE, 0, 0xff & ~(1<<1), WMEN_SIGN1);
          break;

          case 2:
            item = MenuitemCheck(sn->description, "8", TRUE, FALSE, 0, 0xff & ~(1<<2), WMEN_SIGN2);
          break;

          case 3:
            item = MenuitemCheck(sn->description, "9", TRUE, FALSE, 0, 0xff & ~(1<<3), WMEN_SIGN3);
          break;

          default:
            item = MenuitemCheck(sn->description, NULL, TRUE, FALSE, 0, 0xff & ~(1<<i), WMEN_SIGN0+i);
          break;
        }

        if(item != NULL)
        {
          data->MI_SIGNATURES[i] = item;
          DoMethod(data->MI_SIGNATURE, MUIM_Family_AddTail, item);
          // set up the notifications
          DoMethod(item, MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime, data->CY_SIGNATURE, 3, MUIM_Set, MUIA_Cycle_Active, i);
          DoMethod(data->CY_SIGNATURE, MUIM_Notify, MUIA_Cycle_Active, i, data->MI_SIGNATURE, 4, MUIM_SetUData, WMEN_SIGN0+i, MUIA_Menuitem_Checked, TRUE);
        }

        // if the current identity uses a signature then
        // rembember the current one if it matches
        if(activeSig != NULL && sn->id == activeSig->id)
        {
          activeSigIndex = i;
        }
	  }
	  else
	  {
        // maximum reached, bail out
        break;
	  }

	  i++;
    }
  }

  // check the item which belongs to the identity's signature
  set(data->MI_SIGNATURES[activeSigIndex], MUIA_Menuitem_Checked, TRUE);

  RETURN(0);
  return 0;
}

///
/// DECLARE(UpdateIdentities)
// update the identity chooser
DECLARE(UpdateIdentities)
{
  GETDATA;

  ENTER();

  DoMethod(data->CY_FROM, MUIM_IdentityChooser_UpdateIdentities);

  RETURN(0);
  return 0;
}

///
/// DECLARE(HandleAppMessage)
// Hook to catch the MUIA_AppMessage from a workbench icon drop operation
// Note, that this must be a hook or otherwise the "struct AppMessage"
// is not valid anymore at the time this function is executed.
DECLARE(HandleAppMessage) // struct AppMessage *appmsg
{
  ENTER();

  if(msg->appmsg != NULL)
  {
    int i;

    // let's walk through all arguments in the appMessage
    for(i=0; i < msg->appmsg->am_NumArgs; i++)
    {
      struct WBArg *wa = &msg->appmsg->am_ArgList[i];
      char buf[SIZE_PATHFILE];

      NameFromLock(wa->wa_Lock, buf, sizeof(buf));

      if(wa->wa_Name != NULL && strlen(wa->wa_Name) > 0)
      {
        AddPart(buf, (char *)wa->wa_Name, sizeof(buf));

        // call WR_App to let it put in the text of the file
        // to the write window
        DoMethod(obj, METHOD(DroppedFile), buf);
      }
      else
      {
        // open the usual requester, but let it point to the dropped drawer
        DoMethod(obj, METHOD(RequestAttachment), buf);
      }
    }
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(InsertABookTreenode)
DECLARE(InsertABookTreenode) // enum RcptType type, Object *tree, struct MUI_NListtree_TreeNode *tn
{
  struct ABookNode *abn = (struct ABookNode *)msg->tn->tn_User;

  ENTER();

  switch(abn->type)
  {
    case ABNT_USER:
    {
      // insert the address
      DoMethod(obj, METHOD(AddRecipient), msg->type, abn->Alias);
    }
    break;

    case ABNT_LIST:
    {
      char *ptr;

      if((ptr = abn->ListMembers) != NULL)
      {
        while(*ptr != '\0')
        {
          char *nptr;

          if((nptr = strchr(ptr, '\n')) != NULL)
            *nptr = '\0';
          else
            break;

          // insert the address
          DoMethod(obj, METHOD(AddRecipient), msg->type, ptr);

          *nptr = '\n';
          ptr = nptr;

          ptr++;
        }
      }
    }
    break;

    case ABNT_GROUP:
    {
      ULONG pos = MUIV_NListtree_GetEntry_Position_Head;
      struct MUI_NListtree_TreeNode *tn = msg->tn;

      do
      {
        tn = (struct MUI_NListtree_TreeNode *)DoMethod(msg->tree, MUIM_NListtree_GetEntry, tn, pos, MUIV_NListtree_GetEntry_Flag_SameLevel);
        if(tn == NULL)
          break;

        DoMethod(obj, METHOD(InsertABookTreenode), msg->type, msg->tree, tn);

        pos = MUIV_NListtree_GetEntry_Position_Next;
      }
      while(TRUE);
    }
    break;
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(CodesetChanged)
// method that is called when the codeset in the write window is manually
// changed by the user.
DECLARE(CodesetChanged) // char *codesetName
{
  GETDATA;
  ENTER();

  // find the selected codeset and default to the global one if it
  // could not be found
  if((data->wmData->codeset = CodesetsFind(msg->codesetName,
                                           CSA_CodesetList,       G->codesetsList,
                                           CSA_FallbackToDefault, FALSE,
                                           TAG_DONE)) == NULL)
  {
    data->wmData->codeset = G->writeCodeset;
  }

  nnset(data->PO_CODESET, MUIA_CodesetPopup_Codeset, strippedCharsetName(data->wmData->codeset));

  RETURN(0);
  return 0;
}

///
