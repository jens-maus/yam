/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2006 by YAM Open Source Team

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
	if(!DoSuperMethodA(cl, obj, msg))
		return FALSE;
	
	MUI_RequestIDCMP(obj, IDCMP_MOUSEBUTTONS|IDCMP_RAWKEY);
	
	return TRUE;
}

///
/// OVERLOAD(MUIM_Cleanup)
OVERLOAD(MUIM_Cleanup)
{
	MUI_RequestIDCMP(obj, IDCMP_MOUSEBUTTONS|IDCMP_RAWKEY);

	return DoSuperMethodA(cl, obj, msg);
}

///
/// OVERLOAD(MUIM_DragQuery)
OVERLOAD(MUIM_DragQuery)
{
	struct MUIP_DragQuery *d = (struct MUIP_DragQuery *)msg;

	// the attachmentlist either accepts drag requests from the main mail list
	// or from a readmailgroup object
	if(DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_MainMailListGroup_IsMailList, d->obj) == TRUE)
		return MUIV_DragQuery_Accept;
	else if(xget(d->obj, MUIA_AttachmentImage_MailPart) != 0)
		return MUIV_DragQuery_Accept;

	return DoSuperMethodA(cl, obj, msg);
}

///
/// OVERLOAD(MUIM_DragDrop)
OVERLOAD(MUIM_DragDrop)
{
	struct MUIP_DragDrop *d = (struct MUIP_DragDrop *)msg;
	
	if(DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_MainMailListGroup_IsMailList, d->obj) == TRUE)
	{
		struct Attach attach;
		struct Mail *mail;
		int id = MUIV_NList_NextSelected_Start;
		
		do
		{
			DoMethod(d->obj, MUIM_NList_NextSelected, &id);
			if(id == MUIV_NList_NextSelected_End)
				break;
			
			DoMethod(d->obj, MUIM_NList_GetEntry, id, &mail);
			memset(&attach, 0, sizeof(struct Attach));
			GetMailFile(attach.FilePath, NULL, mail);
			stccpy(attach.Description, mail->Subject, SIZE_DEFAULT);
			strcpy(attach.ContentType, "message/rfc822");
			attach.Size = mail->Size;
			attach.IsMIME = TRUE;
			DoMethod(obj, MUIM_NList_InsertSingle, &attach, MUIV_NList_Insert_Bottom);
		}
		while(TRUE);

		return 0;
	}
	else if(xget(d->obj, MUIA_AttachmentImage_MailPart) != 0)
	{
		char tempFile[SIZE_FILE];
		struct Attach attach;
		struct Part *mailPart = (struct Part *)xget(d->obj, MUIA_AttachmentImage_MailPart);

		BusyText(GetStr(MSG_BusyDecSaving), "");

		// make sure the mail part is properly decoded before we add it
		RE_DecodePart(mailPart);

		// clear the attachment structure
		memset(&attach, 0, sizeof(struct Attach));

		// then we create a copy of our decoded file which we can use
		// independent from the object we got the mailPart from
		sprintf(tempFile, "YAMt%08lx.tmp", (ULONG)mailPart);
		strmfp(attach.FilePath, C->TempDir, tempFile);

		// copy the file now
		if(CopyFile(attach.FilePath, NULL, mailPart->Filename, NULL))
		{
			strcpy(attach.Description, mailPart->Description);
			stccpy(attach.ContentType, mailPart->ContentType, SIZE_CTYPE);
			attach.Size = mailPart->Size;
			attach.IsMIME = mailPart->EncodingCode != ENC_UUE;
			attach.IsTemp = TRUE;

			if(mailPart->Name)
				strcpy(attach.Name, mailPart->Name);

			// add the new attachment to the NList
			DoMethod(obj, MUIM_NList_InsertSingle, &attach, MUIV_NList_Insert_Bottom);
		}
		else
			DisplayBeep(NULL);

		BusyEnd();
		return 0;
	}

	return DoSuperMethodA(cl, obj, msg);
}

///

/* Private Functions */

/* Public Methods */
