/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2018 YAM Open Source Team

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

 Superclass:  MUIC_TextEditor
 Description: Our own inherented version of TextEditor.mcc

***************************************************************************/

#include "MailTextEdit_cl.h"

#include <string.h>

#include <proto/codesets.h>
#include <proto/muimaster.h>
#include <mui/NListtree_mcc.h>
#include <mui/TextEditor_mcc.h>

#include "YAM.h"

#include "mui/AddressBookWindow.h"

#include "AddressBook.h"
#include "Config.h"
#include "DynamicString.h"
#include "Locale.h"
#include "MimeTypes.h"
#include "MUIObjects.h"
#include "ParseEmail.h"
#include "Requesters.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  LONG pens[7];
  LONG colorMap[16];

  struct MUI_EventHandlerNode ehnode;

  BOOL eventHandlerAdded;
};
*/

/* EXPORT
// LoadFromFile method flags
#define MUIF_MailTextEdit_LoadFromFile_SetChanged (1<<0)
#define MUIF_MailTextEdit_LoadFromFile_UseStyles  (1<<1)
#define MUIF_MailTextEdit_LoadFromFile_UseColors  (1<<2)
*/

/* Private Function */
/// InsertAddressTreeNode() rec
static void InsertAddressTreeNode(Object *obj, Object *addrObj, struct MUI_NListtree_TreeNode *tn, int i)
{
  struct ABookNode *ab = (struct ABookNode *)(tn->tn_User);
  char address[SIZE_LARGE];

  ENTER();

  switch(ab->type)
  {
    case ABNT_USER:
    {
      // build the address by using the real name and mail address
      BuildAddress(address, sizeof(address), ab->Address, ab->RealName);

      // if we have already inserted an address before we have to seperate them via ", "
      if(i > 0)
        DoMethod(obj, MUIM_TextEditor_InsertText, ", ", MUIV_TextEditor_InsertText_Cursor);

      // insert the address
      DoMethod(obj, MUIM_TextEditor_InsertText, address, MUIV_TextEditor_InsertText_Cursor);
    }
    break;

    case ABNT_LIST:
    {
      char *ptr;

      for(ptr = ab->ListMembers; *ptr != '\0'; ptr++, i++)
      {
        char *nptr;

        if((nptr = strchr(ptr, '\n')) != NULL)
          *nptr = '\0';
        else
          break;

        // if we have already inserted an address before we have to seperate them via ", "
        if(i > 0)
          DoMethod(obj, MUIM_TextEditor_InsertText, ", ", MUIV_TextEditor_InsertText_Cursor);

        // insert the address
        DoMethod(obj, MUIM_TextEditor_InsertText, ptr, MUIV_TextEditor_InsertText_Cursor);

        *nptr = '\n';
        ptr = nptr;
      }
    }
    break;

    case ABNT_GROUP:
    {
      ULONG pos = MUIV_NListtree_GetEntry_Position_Head;
      do
      {
        tn = (struct MUI_NListtree_TreeNode *)DoMethod(addrObj, MUIM_NListtree_GetEntry, tn, pos, MUIV_NListtree_GetEntry_Flag_SameLevel);
        if(tn == NULL)
          break;

        InsertAddressTreeNode(obj, addrObj, tn, i);

        pos = MUIV_NListtree_GetEntry_Position_Next;
        i++;
      }
      while(TRUE);
    }
    break;
  }

  LEAVE();
}

///
/// BuildKeywordString
static char *BuildKeywordString(void)
{
  char *keywords = NULL;
  ULONG i;

  ENTER();

  dstrcpy(&keywords, C->AttachmentKeywords);

  for(i=0; IntMimeTypeArray[i].ContentType != NULL; i++)
  {
    if(IsStrEmpty(IntMimeTypeArray[i].Extension) == FALSE)
    {
      char *copy;

      // split the space separated extensions and build a string of
      // comma separated extensions with leading '.'
      if((copy = strdup(IntMimeTypeArray[i].Extension)) != NULL)
      {
        char *ext = copy;

        do
        {
          char *e;

          if((e = strpbrk(ext, " ")) != NULL)
            *e++ = '\0';

          if(dstrlen(keywords) != 0)
            dstrcat(&keywords, ",");

          dstrcat(&keywords, ".");
          dstrcat(&keywords, ext);

          ext = e;
        }
        while(ext != NULL);

        free(copy);
      }
    }
  }
  D(DBF_GUI, "build keyword string '%s'", keywords);

  RETURN(keywords);
  return keywords;
}

///

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  ENTER();

  if((obj = (Object *)DoSuperMethodA(cl, obj, msg)) != NULL)
  {
    if(GetTagData(MUIA_TextEditor_ReadOnly, FALSE, inittags(msg)) == FALSE &&
       GetTagData(ATTR(CheckKeywords), FALSE, inittags(msg)) == TRUE)
    {
      char *keywords;

      if((keywords = BuildKeywordString()) != NULL)
      {
        set(obj, MUIA_TextEditor_Keywords, keywords);
        dstrfree(keywords);
      }
    }
  }

  RETURN((IPTR)obj);
  return (IPTR)obj;
}

///
/// OVERLOAD(MUIM_DragQuery)
OVERLOAD(MUIM_DragQuery)
{
  struct MUIP_DragDrop *d = (struct MUIP_DragDrop *)msg;
  IPTR result;

  ENTER();

  // only allow drag&drop operations into a writeable texteditor
  // object
  if(xget(obj, MUIA_TextEditor_ReadOnly) == FALSE &&
     G->ABookWinObject != NULL &&
     d->obj == (Object *)xget(G->ABookWinObject, MUIA_AddressBookWindow_Listtree))
  {
    result = MUIV_DragQuery_Accept;
  }
  else
    result = DoSuperMethodA(cl, obj, msg);

  RETURN(result);
  return result;
}

///
/// OVERLOAD(MUIM_DragDrop)
OVERLOAD(MUIM_DragDrop)
{
  struct MUIP_DragDrop *d = (struct MUIP_DragDrop *)msg;
  IPTR result;

  ENTER();

  if(G->ABookWinObject != NULL && d->obj == (Object *)xget(G->ABookWinObject, MUIA_AddressBookWindow_Listtree))
  {
    struct MUI_NListtree_TreeNode *tn = (struct MUI_NListtree_TreeNode *)MUIV_NListtree_NextSelected_Start;
    int i=0;

    do
    {
      DoMethod(d->obj, MUIM_NListtree_NextSelected, &tn);
      if(tn == (struct MUI_NListtree_TreeNode *)MUIV_NListtree_NextSelected_End || tn == NULL)
        break;
      else
        InsertAddressTreeNode(obj, d->obj, tn, i);

      i++;
    }
    while(TRUE);

    result = 0;
  }
  else
    result = DoSuperMethodA(cl, obj, msg);

  RETURN(result);
  return result;
}

///
/// OVERLOAD(MUIM_Setup)
// On the Setup of the TextEditor gadget we prepare the evenhandlernode
// for adding it later on a GoActive Method call.
OVERLOAD(MUIM_Setup)
{
  GETDATA;
  IPTR result;

  ENTER();

  if((result = DoSuperMethodA(cl, obj, msg)))
  {
    data->ehnode.ehn_Priority = 1;
    data->ehnode.ehn_Flags    = 0;
    data->ehnode.ehn_Object   = obj;
    data->ehnode.ehn_Class    = cl;
    data->ehnode.ehn_Events   = IDCMP_RAWKEY;

    DoMethod(_win(obj), MUIM_Window_AddEventHandler, &data->ehnode);
    data->eventHandlerAdded = TRUE;

    // allocate all pens
    data->pens[0] = MUI_ObtainPen(muiRenderInfo(obj), &C->ColoredText, 0);
    data->pens[1] = MUI_ObtainPen(muiRenderInfo(obj), &C->Color1stLevel, 0);
    data->pens[2] = MUI_ObtainPen(muiRenderInfo(obj), &C->Color2ndLevel, 0);
    data->pens[3] = MUI_ObtainPen(muiRenderInfo(obj), &C->Color3rdLevel, 0);
    data->pens[4] = MUI_ObtainPen(muiRenderInfo(obj), &C->Color4thLevel, 0);
    data->pens[5] = MUI_ObtainPen(muiRenderInfo(obj), &C->ColorURL, 0);
    data->pens[6] = MUI_ObtainPen(muiRenderInfo(obj), &C->ColorSignature, 0);

    // fill the colormap, all pens allocated by MUI_ObtainPen() must be converted
    // to true pens by the MUIPEN() macro as all versions of TextEditor.mcc up to
    // 15.39 did not do this. Applying MUIPEN() twice causes no harm.
    data->colorMap[ 6] = MUIPEN(data->pens[0]);
    data->colorMap[ 7] = MUIPEN(data->pens[1]);
    data->colorMap[ 8] = MUIPEN(data->pens[2]);
    data->colorMap[ 9] = MUIPEN(data->pens[3]);
    data->colorMap[10] = MUIPEN(data->pens[4]);
    data->colorMap[11] = MUIPEN(data->pens[5]);
    data->colorMap[12] = MUIPEN(data->pens[6]);

    // set the colormap
    set(obj, MUIA_TextEditor_ColorMap, data->colorMap);
  }

  RETURN(result);
  return result;
}

///
/// OVERLOAD(MUIM_Cleanup)
// On a Cleanup we have to remove the EventHandler
OVERLOAD(MUIM_Cleanup)
{
  GETDATA;
  IPTR result;
  size_t i;

  ENTER();

  if(data->eventHandlerAdded == TRUE)
  {
    DoMethod(_win(obj), MUIM_Window_RemEventHandler, &data->ehnode);
    data->eventHandlerAdded = FALSE;
  }

  // unset the colormap
  set(obj, MUIA_TextEditor_ColorMap, NULL);

  // release all pens of our own colorMap
  for(i = 0; i < ARRAY_SIZE(data->pens); i++)
  {
    MUI_ReleasePen(muiRenderInfo(obj), data->pens[i]);
    data->pens[i] = -1;
  }

  result = DoSuperMethodA(cl, obj, msg);

  RETURN(result);
  return result;
}

///
/// OVERLOAD(MUIM_HandleEvent)
// We use HandleEvent to implement our neat tiny feature that on a press of
// RAMIGA+DEL while a multiline text is marked YAM deletes the text and
// inserts the famous [...] substitution.
OVERLOAD(MUIM_HandleEvent)
{
  struct IntuiMessage *imsg;

  if((imsg = ((struct MUIP_HandleEvent *)msg)->imsg))
  {
    if(imsg->Class == IDCMP_RAWKEY)
    {
      if(imsg->Code == IECODE_DEL)
      {
        if(isFlagSet(imsg->Qualifier, IEQUALIFIER_RCOMMAND) &&
           !xget(obj, MUIA_TextEditor_ReadOnly))
        {
          ULONG ret;
          ULONG x1, y1, x2, y2;

          // let`s check first if a multiline block is marked or not
          if(DoMethod(obj, MUIM_TextEditor_BlockInfo, &x1, &y1, &x2, &y2) && y2-y1 >= 1)
          {
            // then we first clear the qualifier so that the real
            // TextEditor HandleEvent method treats this imsg as a normal DEL pressed imsg
            clearFlag(imsg->Qualifier, IEQUALIFIER_RCOMMAND);
            ret = DoSuperMethodA(cl, obj, msg);

            // Now that the marked text is cleared we can insert our great [...]
            // snip text ;)
            DoMethod(obj, MUIM_TextEditor_InsertText, "[...]\n", MUIV_TextEditor_InsertText_Cursor);

            return ret;
          }
        }
      }
    }
  }

  return DoSuperMethodA(cl, obj, msg);
}

///
/// OVERLOAD(MUIM_TextEditor_HandleError)
OVERLOAD(MUIM_TextEditor_HandleError)
{
  const char *errortxt = NULL;

  ENTER();

  SHOWVALUE(DBF_GUI, ((struct MUIP_TextEditor_HandleError *)msg)->errorcode);

  switch(((struct MUIP_TextEditor_HandleError *)msg)->errorcode)
  {
    case Error_ClipboardIsEmpty:
    case Error_ClipboardIsNotFTXT:
    case Error_NoAreaMarked:
    case Error_NothingToRedo:
    case Error_NothingToUndo:
      // nothing but DisplayBeep()
    break;

    case Error_NotEnoughUndoMem:
      errortxt = tr(MSG_WR_ErrorNotEnoughUndoMem);
    break;
  }

  if(errortxt)
    MUI_Request(_app(obj), _win(obj), MUIF_NONE, NULL, tr(MSG_OkayReq), errortxt);
  else
    DisplayBeep(_screen(obj));

  LEAVE();
  return DoSuperMethodA(cl, obj, msg);
}

///

/* Private Functions */

/* Public Methods */
/// DECLARE(LoadFromFile)
//  Loads a text from a file
DECLARE(LoadFromFile) // const char *file, struct codeset *srcCodeset, ULONG flags
{
  BOOL result = FALSE;
  char *text;
  size_t textlen;

  ENTER();

  if((text = FileToBuffer(msg->file, &textlen)) != NULL)
  {
    char *dstText;
    ULONG dstLen = 0;
    BOOL converted = FALSE;

    // lets convert to the specified srcCodeset if set and different
    // from the localCodeset
    if(msg->srcCodeset != NULL && stricmp(msg->srcCodeset->name, G->localCodeset->name) != 0)
    {
      D(DBF_MAIL, "convert file content of '%s' from '%s' to '%s'", msg->file, msg->srcCodeset->name, G->localCodeset->name);

      // convert from the srcCodeset to the localCodeset
      dstText = CodesetsConvertStr(CSA_SourceCodeset,   msg->srcCodeset,
                                   CSA_DestCodeset,     G->localCodeset,
                                   CSA_Source,          text,
                                   CSA_SourceLen,       textlen,
                                   CSA_DestLenPtr,      &dstLen,
                                   CSA_MapForeignChars, C->MapForeignChars,
                                   TAG_DONE);

      if(dstText != NULL)
        converted = TRUE;
      else
        dstLen = 0;
    }
    else
    {
      dstText = text;
      dstLen = strlen(text);
    }

    // check if operations succeeded
    if(dstLen > 0)
    {
      char *parsedText;

      // parse the text and do some highlighting and stuff
      if((parsedText = ParseEmailText(dstText, FALSE, isFlagSet(msg->flags, MUIF_MailTextEdit_LoadFromFile_UseStyles),
                                                      isFlagSet(msg->flags, MUIF_MailTextEdit_LoadFromFile_UseColors))) != NULL)
      {
        // set the new text and tell the editor that its content has changed
        xset(obj, MUIA_TextEditor_Contents,   parsedText,
                  MUIA_TextEditor_HasChanged, isFlagSet(msg->flags, MUIF_MailTextEdit_LoadFromFile_SetChanged));

        dstrfree(parsedText);

        result = TRUE;
      }
    }

    if(converted == TRUE)
      CodesetsFreeA(dstText, NULL);

    free(text);
  }

  RETURN(result);
  return result;
}

///
/// DECLARE(SaveToFile)
//  Saves the contents to a file
DECLARE(SaveToFile) // const char *file, struct codeset *dstCodeset
{
  BOOL result = FALSE;
  FILE *fh;

  ENTER();

  if((fh = fopen(msg->file, "w")) != NULL)
  {
    char *text = (char *)DoMethod(obj, MUIM_TextEditor_ExportText);
    if(text != NULL)
    {
      char *dstText;
      ULONG dstLen = 0;
      BOOL converted = FALSE;

      // lets convert to the specified dstCodeset if set and different
      // from the localCodeset
      if(msg->dstCodeset != NULL && stricmp(msg->dstCodeset->name, G->localCodeset->name) != 0)
      {
        D(DBF_MAIL, "convert file content of '%s' from '%s' to '%s'", msg->file, G->localCodeset->name, msg->dstCodeset->name);

        // convert from the readCharset to dstCodeset (e.g. selected in write window)
        dstText = CodesetsConvertStr(CSA_SourceCodeset,   G->localCodeset,
                                     CSA_DestCodeset,     msg->dstCodeset,
                                     CSA_Source,          text,
                                     CSA_SourceLen,       strlen(text),
                                     CSA_DestLenPtr,      &dstLen,
                                     CSA_MapForeignChars, C->MapForeignChars,
                                     TAG_DONE);

        if(dstText != NULL)
          converted = TRUE;
        else
          dstLen = 0;
      }
      else
      {
        dstText = text;
        dstLen = strlen(text);
      }

      // check if operations succeeded
      if(dstLen > 0)
      {
        // write out the whole text to the file
        if(fwrite(dstText, dstLen, 1, fh) == 1)
          result = TRUE;
      }

      if(converted == TRUE)
        CodesetsFreeA(dstText, NULL);

      // the exported text must be freed using FreeVec()
      FreeVec(text);
    }

    fclose(fh);
  }

  RETURN(result);
  return result;
}

///
