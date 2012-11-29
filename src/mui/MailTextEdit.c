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

 Superclass:  MUIC_TextEditor
 Description: Our own inherented version of TextEditor.mcc

***************************************************************************/

#include "MailTextEdit_cl.h"

#include <string.h>
#include <proto/muimaster.h>
#include <mui/TextEditor_mcc.h>

#include "YAM.h"
#include "YAM_addressbook.h"
#include "YAM_addressbookEntry.h"
#include "YAM_config.h"

#include "Locale.h"
#include "MUIObjects.h"
#include "ParseEmail.h"
#include "Requesters.h"
#include "StrBuf.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *slider;
  LONG pens[7];
  LONG colorMap[16];

  struct MUI_EventHandlerNode ehnode;

  BOOL eventHandlerAdded;
};
*/

/* EXPORT
#define MUIF_MailTextEdit_LoadFromFile_SetChanged (1<<0)
#define MUIF_MailTextEdit_LoadFromFile_UseStyles  (1<<1)
#define MUIF_MailTextEdit_LoadFromFile_UseColors  (1<<2)
*/

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  ENTER();

  if((obj = (Object *)DoSuperMethodA(cl, obj, msg)))
  {
    GETDATA;
    struct TagItem *tags = inittags(msg), *tag;

    while((tag = NextTagItem((APTR)&tags)) != NULL)
    {
      switch(tag->ti_Tag)
      {
        // we also catch foreign attributes
        case MUIA_TextEditor_Slider: data->slider = (Object *)tag->ti_Data; break;
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
  struct MUIP_DragDrop *drop_msg = (struct MUIP_DragDrop *)msg;

  return (ULONG)(drop_msg->obj == G->AB->GUI.LV_ADDRESSES);
}

///
/// OVERLOAD(MUIM_DragDrop)
OVERLOAD(MUIM_DragDrop)
{
  struct MUIP_DragDrop *drop_msg = (struct MUIP_DragDrop *)msg;

  if(drop_msg->obj == G->AB->GUI.LV_ADDRESSES)
  {
    struct MUI_NListtree_TreeNode *tn;

    if((tn = (struct MUI_NListtree_TreeNode *)xget(drop_msg->obj, MUIA_NListtree_Active)))
    {
      struct ABEntry *ab = (struct ABEntry *)(tn->tn_User);

      if(ab->Type != AET_GROUP)
      {
        char address[SIZE_LARGE];

        BuildAddress(address, sizeof(address), ab->Address, ab->RealName);
        DoMethod(obj, MUIM_TextEditor_InsertText, address, MUIV_TextEditor_InsertText_Cursor);
      }
    }
  }

  return DoSuperMethodA(cl, obj, msg);
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
  int i;

  ENTER();

  if(data->eventHandlerAdded == TRUE)
  {
    DoMethod(_win(obj), MUIM_Window_RemEventHandler, &data->ehnode);
    data->eventHandlerAdded = FALSE;
  }

  // unset the colormap
  set(obj, MUIA_TextEditor_ColorMap, NULL);

  // release all pens of our own colorMap
  for(i = 0; i <= ARRAY_SIZE(data->pens); i++)
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
    DisplayBeep(NULL);

  LEAVE();
  return DoSuperMethodA(cl, obj, msg);
}

///

/* Private Functions */

/* Public Methods */
/// DECLARE(LoadFromFile)
//  Loads a text from a file
DECLARE(LoadFromFile) // const char *file, ULONG flags
{
  BOOL result = FALSE;
  char *text;

  ENTER();

  if((text = FileToBuffer(msg->file)) != NULL)
  {
    char *parsedText;

    // parse the text and do some highlighting and stuff
    if((parsedText = ParseEmailText(text, FALSE, isFlagSet(msg->flags, MUIF_MailTextEdit_LoadFromFile_UseStyles), isFlagSet(msg->flags, MUIF_MailTextEdit_LoadFromFile_UseColors))) != NULL)
    {
      // set the new text and tell the editor that its content has changed
      xset(obj, MUIA_TextEditor_Contents,   parsedText,
                MUIA_TextEditor_HasChanged, isFlagSet(msg->flags, MUIF_MailTextEdit_LoadFromFile_SetChanged));

      FreeStrBuf(parsedText);

      result = TRUE;
    }

    free(text);
  }

  RETURN(result);
  return result;
}

///
/// DECLARE(SaveToFile)
//  Saves the contents to a file
DECLARE(SaveToFile) // const char *file
{
  BOOL result = FALSE;
  FILE *fh;

  ENTER();

  if((fh = fopen(msg->file, "w")) != NULL)
  {
    char *text = (char *)DoMethod(obj, MUIM_TextEditor_ExportText);

    // write out the whole text to the file
    if(fwrite(text, strlen(text), 1, fh) == 1)
      result = TRUE;

    // the exported text must be freed using FreeVec()
    FreeVec(text);
    fclose(fh);
  }

  RETURN(result);
  return result;
}

///
