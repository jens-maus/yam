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

 Superclass:  MUIC_NList
 Description: NList class of the write window's attachment list

***************************************************************************/

#include "WriteAttachmentList_cl.h"

#include "YAM_mainFolder.h"

#include "MUIObjects.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  short dummy;
};
*/

/* Overloaded Methods */
/// OVERLOAD(MUIM_Setup)
OVERLOAD(MUIM_Setup)
{
  IPTR result;

  ENTER();

  if((result = DoSuperMethodA(cl, obj, msg)))
  {
    MUI_RequestIDCMP(obj, IDCMP_MOUSEBUTTONS|IDCMP_RAWKEY);
  }

  RETURN(result);
  return result;
}

///
/// OVERLOAD(MUIM_Cleanup)
OVERLOAD(MUIM_Cleanup)
{
  IPTR result;

  ENTER();

  MUI_RequestIDCMP(obj, IDCMP_MOUSEBUTTONS|IDCMP_RAWKEY);

  result = DoSuperMethodA(cl, obj, msg);

  RETURN(result);
  return result;
}

///
/// OVERLOAD(MUIM_DragQuery)
OVERLOAD(MUIM_DragQuery)
{
  struct MUIP_DragQuery *d = (struct MUIP_DragQuery *)msg;
  IPTR result;

  ENTER();

  // the attachmentlist either accepts drag requests from the main mail list
  // or from a readmailgroup object
  if(DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_MainMailListGroup_IsMailList, d->obj) == TRUE)
    result = MUIV_DragQuery_Accept;
  else if(xget(d->obj, MUIA_AttachmentImage_MailPart) != 0)
    result = MUIV_DragQuery_Accept;
  else
    result = DoSuperMethodA(cl, obj, msg);

  RETURN(result);
  return(result);
}

///
/// OVERLOAD(MUIM_DragDrop)
OVERLOAD(MUIM_DragDrop)
{
  struct MUIP_DragDrop *d = (struct MUIP_DragDrop *)msg;
  IPTR result;

  ENTER();

  if(DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_MainMailListGroup_IsMailList, d->obj) == TRUE)
  {
    int id = MUIV_NList_NextSelected_Start;

    do
    {
      struct Mail *mail=NULL;

      DoMethod(d->obj, MUIM_NList_NextSelected, &id);
      if(id == MUIV_NList_NextSelected_End)
        break;

      DoMethod(d->obj, MUIM_NList_GetEntry, id, &mail);
      if(mail != NULL)
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
          DoMethod(obj, MUIM_NList_InsertSingle, &attach, MUIV_NList_Insert_Bottom);
        }
        else
          E(DBF_MAIL, "unpacking of file '%s' failed!", filename);
      }
      else
        break;
    }
    while(TRUE);

    result = 0;
  }
  else if(xget(d->obj, MUIA_AttachmentImage_MailPart) != 0)
  {
    char tempFile[SIZE_FILE];
    struct Attach attach;
    struct Part *mailPart = (struct Part *)xget(d->obj, MUIA_AttachmentImage_MailPart);

    BusyText(tr(MSG_BusyDecSaving), "");

    // make sure the mail part is properly decoded before we add it
    RE_DecodePart(mailPart);

    // clear the attachment structure
    memset(&attach, 0, sizeof(struct Attach));

    // then we create a copy of our decoded file which we can use
    // independent from the object we got the mailPart from
    snprintf(tempFile, sizeof(tempFile), "YAMt%08x.tmp", (unsigned int)mailPart);
    AddPath(attach.FilePath, C->TempDir, tempFile, sizeof(attach.FilePath));

    // copy the file now
    if(CopyFile(attach.FilePath, NULL, mailPart->Filename, NULL))
    {
      strlcpy(attach.Description, mailPart->Description, sizeof(attach.Description));
      strlcpy(attach.ContentType, mailPart->ContentType, sizeof(attach.ContentType));
      attach.Size = mailPart->Size;
      attach.IsMIME = mailPart->EncodingCode != ENC_UUE;
      attach.IsTemp = TRUE;

      if(mailPart->Name)
        strlcpy(attach.Name, mailPart->Name, sizeof(attach.Name));

      // add the new attachment to the NList
      DoMethod(obj, MUIM_NList_InsertSingle, &attach, MUIV_NList_Insert_Bottom);
    }
    else
      DisplayBeep(_screen(obj));

    BusyEnd();
    result = 0;
  }
  else
  {
    // let the superclass decide
    result = DoSuperMethodA(cl, obj, msg);
  }

  RETURN(result);
  return result;
}

///

/* Private Functions */

/* Public Methods */

