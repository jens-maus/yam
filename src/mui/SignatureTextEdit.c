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

 Superclass:  MUIC_MailTextEdit
 Description: Edit signature texts

***************************************************************************/

#include "SignatureTextEdit_cl.h"

#include <string.h>

#include <proto/dos.h>
#include <mui/TextEditor_mcc.h>

#include "YAM_config.h"

#include "DynamicString.h"
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
          // switch read-only/edit mode
          set(obj, ATTR(UseSignatureFile), data->sigNode->useSignatureFile);
        }
      }
      break;

      case ATTR(UseSignatureFile):
      {
        if(tag->ti_Data == FALSE)
        {
          // switch to edit mode if no signature file is used
          SetSuperAttrs(cl, obj, MUIA_TextEditor_ReadOnly, FALSE,
                                 MUIA_TextEditor_ActiveObjectOnClick, TRUE,
                                 TAG_DONE);
        }
        else
        {
          // switch to read-write mode if a signature file is used
          SetSuperAttrs(cl, obj, MUIA_TextEditor_ReadOnly, TRUE,
                                 TAG_DONE);
        }
      }
    }
  }

  return DoSuperMethodA(cl, obj, msg);
}

///
/// OVERLOAD(OM_GET)
OVERLOAD(OM_GET)
{
  GETDATA;
  IPTR *store = ((struct opGet *)msg)->opg_Storage;

  switch(((struct opGet *)msg)->opg_AttrID)
  {
    case ATTR(SignatureNode): *store = (IPTR)data->sigNode; return TRUE;
  }

  return DoSuperMethodA(cl, obj, msg);
}

///

/* Private Functions */

/* Public Methods */
/// DECLARE(SetSignatureText)
DECLARE(SetSignatureText) // const char *sigText
{
  GETDATA;
  char *parsedSig;

  ENTER();

  // refresh ourself with the new signature text
  if(msg->sigText != NULL && (parsedSig = ParseEmailText(msg->sigText, FALSE, TRUE, TRUE)) != NULL)
  {
    BOOL modified;

    if(data->sigNode != NULL && data->sigNode->signature != NULL)
      modified = (strcmp(msg->sigText, data->sigNode->signature) != 0);
    else
      modified = TRUE;

    xset(obj, MUIA_TextEditor_Contents, parsedSig,
              MUIA_TextEditor_HasChanged, modified);

    dstrfree(parsedSig);
  }
  else
  {
    if(msg->sigText != NULL)
      W(DBF_CONFIG, "couldn't load signature '%s' in texteditor", SafeStr(msg->sigText));

    xset(obj, MUIA_TextEditor_Contents, "",
              MUIA_TextEditor_HasChanged, FALSE);
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(EditExternally)
DECLARE(EditExternally)
{
  ENTER();

  if(C->Editor[0] != '\0')
  {
    struct TempFile *tf;

    if((tf = OpenTempFile(NULL)) != NULL)
    {
      char *editor = NULL;
      char *buffer;

      // export the signature text to a temporaty file
      DoMethod(obj, MUIM_MailTextEdit_SaveToFile, tf->Filename, NULL);

      // launch the external editor and wait until it is
      // finished...
      if(asprintf(&editor, "%s \"%s\"", C->Editor, tf->Filename) >= 0)
      {
        // launch the external editor synchronously (wait until it returns)
        LaunchCommand(editor, 0, OUT_NIL);
        free(editor);

        // refresh the signature in the internal editor after the external is finished
        if((buffer = FileToBuffer(tf->Filename)) != NULL)
        {
          DoMethod(obj, METHOD(SetSignatureText), buffer);
          free(buffer);
        }
      }

      CloseTempFile(tf);
    }
  }

  RETURN(0);
  return 0;
}

///
