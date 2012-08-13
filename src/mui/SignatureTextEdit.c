/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2012 YAM Open Source Team

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

 Superclass:  MUIC_MailTextEdit
 Description: Edit signature texts

***************************************************************************/

#include "SignatureTextEdit_cl.h"

#include <proto/dos.h>
#include <mui/TextEditor_mcc.h>

#include "YAM_config.h"

#include "MailTextEdit.h"
#include "MUIObjects.h"
#include "ParseEmail.h"
#include "Signature.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  struct SignatureNode *sigNode;
};
*/

/* Overloaded Methods */
/// OVERLOAD(OM_SET)
OVERLOAD(OM_SET)
{
  GETDATA;
  struct TagItem *tags = inittags(msg), *tag;

  while((tag = NextTagItem((APTR)&tags)) != NULL)
  {
    switch(tag->ti_Tag)
    {
	  case ATTR(SignatureNode):
	  {
        if(data->sigNode != NULL && xget(obj, MUIA_TextEditor_HasChanged) == TRUE)
        {
          char *sig;

          // obtain the signature text and save it for the previous signature node
          if((sig = (char *)DoMethod(obj, MUIM_TextEditor_ExportText)) != NULL)
          {
            // replace the old text with the current one
            free(data->sigNode->signature);
            data->sigNode->signature = strdup(sig);
            FreeVec(sig);
          }
        }

        // remember the new signature node
	    data->sigNode = (struct SignatureNode *)tag->ti_Data;

        if(data->sigNode != NULL)
        {
          // import the new signature text
          DoMethod(obj, METHOD(SetSignatureText), data->sigNode->signature);
        }
	  }
	  break;
    }
  }

  return DoSuperMethodA(cl, obj, msg);
}

/* Private Functions */

/* Public Methods */
/// DECLARE(SetSignatureText)
DECLARE(SetSignatureText) // const char *sigText
{
  GETDATA;
  char *parsedSig;

  ENTER();

  // refresh outself with the new signature text
  if(msg->sigText != NULL && (parsedSig = ParseEmailText(msg->sigText, FALSE, TRUE, TRUE)) != NULL)
  {
    BOOL modified;

    if(data->sigNode != NULL && data->sigNode->signature != NULL)
      modified = (strcmp(msg->sigText, data->sigNode->signature) != 0);
    else
      modified = TRUE;

	xset(obj, MUIA_TextEditor_Contents, parsedSig,
			  MUIA_TextEditor_HasChanged, modified);

	free(parsedSig);
  }
  else
  {
	W(DBF_CONFIG, "couldn't load signature '%s' in texteditor", SafeStr(msg->sigText));

	DoMethod(obj, MUIM_TextEditor_ClearText);
  }

  LEAVE();
  return 0;
}

///
/// DECLARE(EditExternally)
DECLARE(EditExternally)
{
  ENTER();

  if(C->Editor[0] != '\0')
  {
    char editor[SIZE_LARGE];
    char *buffer;

    // export the signature text to a temporaty file
    DoMethod(obj, MUIM_MailTextEdit_SaveToFile, "T:tempsignature");

    // launch the external editor and wait until it is
    // finished...
    snprintf(editor, sizeof(editor), "%s \"T:tempsignature\"", C->Editor);
    LaunchCommand(editor, FALSE, OUT_NIL);

    // refresh the signature in the internal editor after the external is finished
    if((buffer = FileToBuffer("T:tempsignature")) != NULL)
    {
      DoMethod(obj, METHOD(SetSignatureText), buffer);
      free(buffer);
    }

    // delete the temporary signature file again
    if(DeleteFile("T:tempsignature") == 0)
      AddZombieFile("T:tempsignature");
  }

  LEAVE();
  return 0;
}

///
