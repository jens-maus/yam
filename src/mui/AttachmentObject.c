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
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.   See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 YAM Official Support Site : http://www.yam.ch
 YAM OpenSource project     : http://sourceforge.net/projects/yamos/

 $Id$

 Superclass:  MUIC_Group
 Description: Custom class to manage the attachments of a mail

 Credits: This class was highly inspired by the similar attachment group &
          image functionality available in Thunderbird and SimpleMail. Large
          code portions where borrowed by the iconclass implementation of
          SimpleMail to allow loading of the default icons via icon.library
          and supporting Drag&Drop on the workbench. Thanks sba! :)

***************************************************************************/

#include "AttachmentObject_cl.h"

#include "YAM_mainFolder.h"

#include "MUIObjects.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *imageObject;
  Object *textObject;
  struct Part *mailPart;
};
*/

#define BORDER    2 // border around our object
#define SPACING   2 // pixels taken as space between our images/text
#define TEXTROWS  3 // how many text rows does a attachmentimage normally have?

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  Object *imageObject;
  Object *textObject;
  struct Part *mailPart;

  ENTER();

  mailPart = (struct Part *)GetTagData(MUIA_AttachmentObject_MailPart, (ULONG)NULL, inittags(msg));

  if((obj = DoSuperNew(cl, obj,
                        MUIA_Group_Horiz,   TRUE,
                        MUIA_Group_Spacing, 0,
                        Child, imageObject = AttachmentImageObject,
                          MUIA_CycleChain,               TRUE,
                          MUIA_AttachmentImage_MailPart, mailPart,
                        End,
                        Child, textObject = TextObject,
                          MUIA_Text_SetMax, FALSE,
                          MUIA_Font,        MUIV_Font_Tiny,
                        End,
                      TAG_MORE, inittags(msg))) != NULL)
  {
    GETDATA;

    data->imageObject = imageObject;
    data->textObject = textObject;
    data->mailPart = mailPart;
  }

  RETURN((ULONG)obj);
  return (ULONG)obj;
}

///
/// OVERLOAD(OM_GET)
OVERLOAD(OM_GET)
{
  GETDATA;
  ULONG *store = ((struct opGet *)msg)->opg_Storage;

  switch(((struct opGet *)msg)->opg_AttrID)
  {
    ATTR(ImageObject) : *store = (ULONG)data->imageObject; return TRUE;
    ATTR(MailPart)    : *store = (ULONG)data->mailPart;    return TRUE;
  }

  return DoSuperMethodA(cl, obj, msg);
}

///
/// OVERLOAD(MUIM_Setup)
OVERLOAD(MUIM_Setup)
{
  GETDATA;
  ULONG result = 0;

  ENTER();

  if(data->mailPart != NULL && (result = DoSuperMethodA(cl, obj, msg)) != 0)
  {
	char buffer[SIZE_DEFAULT];
	char sizeBuffer[SIZE_DEFAULT];

    // first line: the
    if(isAlternativePart(data->mailPart))
    {
      strlcpy(buffer, MUIX_I "multipart/alternative" MUIX_N, sizeof(buffer));
    }
    else
    {
      if(data->mailPart->Name[0] != '\0')
        strlcpy(buffer, data->mailPart->Name, sizeof(buffer));
      else
        strlcpy(buffer, data->mailPart->Description, sizeof(buffer));
    }
    strlcat(buffer, "\n", sizeof(buffer));

    // calculate the sizeLabelLen
    if(isDecoded(data->mailPart))
    {
      FormatSize(data->mailPart->Size, sizeBuffer, sizeof(sizeBuffer), SF_AUTO);
    }
    else
    {
      sizeBuffer[0] = '~';
      FormatSize(data->mailPart->Size, &sizeBuffer[1], sizeof(sizeBuffer)-1, SF_AUTO);
    }
    strlcat(buffer, sizeBuffer, sizeof(buffer));
    strlcat(buffer, "\n", sizeof(buffer));

    strlcat(buffer, DescribeCT(data->mailPart->ContentType), sizeof(buffer));

    set(data->textObject, MUIA_Text_Contents, buffer);

    xset(data->imageObject, MUIA_AttachmentImage_MaxHeight, _font(obj) ? TEXTROWS*_font(obj)->tf_YSize+4 : 0,
                            MUIA_AttachmentImage_MaxWidth,  _font(obj) ? TEXTROWS*_font(obj)->tf_YSize+4 : 0);
  }

  RETURN(result);
  return result;
}

///

/* Private Functions */

/* Public Methods */
