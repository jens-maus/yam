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
 Description: NList class to display variable placeholders

***************************************************************************/

#include "PlaceholderList_cl.h"

#include <string.h>
#include <mui/NList_mcc.h>

#include "YAM_main.h"
#include "YAM_utilities.h"

#include "Locale.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  char placeholder[8];
  char description[SIZE_LARGE];
};
*/

/* EXPORT
enum VariablePopMode
{
  VPM_FORWARD=0,
  VPM_REPLYHELLO,
  VPM_REPLYINTRO,
  VPM_REPLYBYE,
  VPM_ARCHIVE,
  VPM_MAILSTATS,
  VPM_SCRIPTS,
  VPM_MIME_DEFVIEWER,
  VPM_MIME_COMMAND,
};
*/

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  enum VariablePopMode mode;

  ENTER();

  mode = (enum VariablePopMode)GetTagData(ATTR(Mode), VPM_FORWARD, inittags(msg));

  if((obj = DoSuperNew(cl, obj,

    InputListFrame,
    MUIA_NList_AdjustHeight, TRUE,
    MUIA_NList_Format, ",",

    TAG_MORE, inittags(msg))) != NULL)
  {
    switch(mode)
    {
      case VPM_FORWARD:
      case VPM_REPLYHELLO:
      case VPM_REPLYINTRO:
      case VPM_REPLYBYE:
      {
        DoMethod(obj, MUIM_NList_InsertSingle, tr(MSG_CO_LineBreak), MUIV_NList_Insert_Bottom);
        DoMethod(obj, MUIM_NList_InsertSingle, mode != VPM_FORWARD ? tr(MSG_CO_RecptName) : tr(MSG_CO_ORecptName), MUIV_NList_Insert_Bottom);
        DoMethod(obj, MUIM_NList_InsertSingle, mode != VPM_FORWARD ? tr(MSG_CO_RecptFirstname) : tr(MSG_CO_ORecptFirstname), MUIV_NList_Insert_Bottom);
        DoMethod(obj, MUIM_NList_InsertSingle, mode != VPM_FORWARD ? tr(MSG_CO_RecptAddress) : tr(MSG_CO_ORecptAddress), MUIV_NList_Insert_Bottom);
        DoMethod(obj, MUIM_NList_InsertSingle, tr(MSG_CO_SenderName), MUIV_NList_Insert_Bottom);
        DoMethod(obj, MUIM_NList_InsertSingle, tr(MSG_CO_SenderFirstname), MUIV_NList_Insert_Bottom);
        DoMethod(obj, MUIM_NList_InsertSingle, tr(MSG_CO_SenderAddress), MUIV_NList_Insert_Bottom);
        DoMethod(obj, MUIM_NList_InsertSingle, tr(MSG_CO_SenderSubject), MUIV_NList_Insert_Bottom);
        DoMethod(obj, MUIM_NList_InsertSingle, tr(MSG_CO_SenderRFCDateTime), MUIV_NList_Insert_Bottom);
        DoMethod(obj, MUIM_NList_InsertSingle, tr(MSG_CO_SenderDate), MUIV_NList_Insert_Bottom);
        DoMethod(obj, MUIM_NList_InsertSingle, tr(MSG_CO_SenderTime), MUIV_NList_Insert_Bottom);
        DoMethod(obj, MUIM_NList_InsertSingle, tr(MSG_CO_SenderTimeZone), MUIV_NList_Insert_Bottom);
        DoMethod(obj, MUIM_NList_InsertSingle, tr(MSG_CO_SenderDOW), MUIV_NList_Insert_Bottom);
        DoMethod(obj, MUIM_NList_InsertSingle, tr(MSG_CO_SenderMsgID), MUIV_NList_Insert_Bottom);

        // depending on the mode we have the "CompleteHeader" feature or not.
        if(mode == VPM_FORWARD || mode == VPM_REPLYINTRO)
          DoMethod(obj, MUIM_NList_InsertSingle, tr(MSG_CO_CompleteHeader), MUIV_NList_Insert_Bottom);
      }
      break;

      case VPM_ARCHIVE:
      {
        DoMethod(obj, MUIM_NList_InsertSingle, tr(MSG_CO_ArchiveName), MUIV_NList_Insert_Bottom);
        DoMethod(obj, MUIM_NList_InsertSingle, tr(MSG_CO_ArchiveFiles), MUIV_NList_Insert_Bottom);
        DoMethod(obj, MUIM_NList_InsertSingle, tr(MSG_CO_ArchiveFilelist), MUIV_NList_Insert_Bottom);
      }
      break;

      case VPM_MAILSTATS:
      {
        DoMethod(obj, MUIM_NList_InsertSingle, tr(MSG_CO_NEWMSGS), MUIV_NList_Insert_Bottom);
        DoMethod(obj, MUIM_NList_InsertSingle, tr(MSG_CO_UNREADMSGS), MUIV_NList_Insert_Bottom);
        DoMethod(obj, MUIM_NList_InsertSingle, tr(MSG_CO_TOTALMSGS), MUIV_NList_Insert_Bottom);
        DoMethod(obj, MUIM_NList_InsertSingle, tr(MSG_CO_DELMSGS), MUIV_NList_Insert_Bottom);
        DoMethod(obj, MUIM_NList_InsertSingle, tr(MSG_CO_SENTMSGS), MUIV_NList_Insert_Bottom);
      }
      break;

      case VPM_SCRIPTS:
      {
        // nothing to insert here, this is done externally depending on the type of the script
      }
      break;

      case VPM_MIME_DEFVIEWER:
      case VPM_MIME_COMMAND:
      {
        DoMethod(obj, MUIM_NList_InsertSingle, tr(MSG_CO_MIMECMD_PARAMETER), MUIV_NList_Insert_Bottom);
        DoMethod(obj, MUIM_NList_InsertSingle, tr(MSG_CO_MIMECMD_PUBSCREEN), MUIV_NList_Insert_Bottom);
      }
      break;
    }
  }

  RETURN((IPTR)obj);
  return (IPTR)obj;
}

///
/// OVERLOAD(MUIM_NList_Display)
OVERLOAD(MUIM_NList_Display)
{
  struct MUIP_NList_Display *ndm = (struct MUIP_NList_Display *)msg;
  char *entry = (char *)ndm->entry;

  ENTER();

  if(entry != NULL)
  {
    GETDATA;
    char *src = entry;
    char *dst = data->placeholder;
    size_t i;

    i = 0;
    // copy the string until the first space character occurs
    while(*src != '\0' && *src != ' ' && i < sizeof(data->placeholder) - 1)
    {
      *dst++ = *src++;
      i++;
    }
    // NUL terminate the string
    *dst = '\0';

    // skip the spaces
    while(*src != '\0' && *src == ' ')
      src++;

    // finally copy the rest of the string to the description text
    strlcpy(data->description, src, sizeof(data->description));

    ndm->strings[0] = data->placeholder;
    ndm->strings[1] = data->description;
  }

  LEAVE();
  return 0;
}

///

/* Private Functions */

/* Public Methods */
/// DECLARE(SetScriptEntry)
// set the placeholder entries for a script
DECLARE(SetScriptEntry) // enum Macro macro
{
  ENTER();

  // clear the list first
  DoMethod(obj, MUIM_NList_Clear);

  switch(msg->macro)
  {
    case MACRO_MEN0:
    case MACRO_MEN1:
    case MACRO_MEN2:
    case MACRO_MEN3:
    case MACRO_MEN4:
    case MACRO_MEN5:
    case MACRO_MEN6:
    case MACRO_MEN7:
    case MACRO_MEN8:
    case MACRO_MEN9:
    case MACRO_STARTUP:
    case MACRO_QUIT:
    case MACRO_PRESEND:
    case MACRO_POSTSEND:
    case MACRO_PREFILTER:
    case MACRO_POSTFILTER:
    default:
      // nothing
    break;

    case MACRO_PREGET:
      DoMethod(obj, MUIM_NList_InsertSingle, tr(MSG_CO_SCRIPTS_PREGET), MUIV_NList_Insert_Bottom);
    break;

    case MACRO_POSTGET:
      DoMethod(obj, MUIM_NList_InsertSingle, tr(MSG_CO_SCRIPTS_POSTGET), MUIV_NList_Insert_Bottom);
    break;

    case MACRO_NEWMSG:
      DoMethod(obj, MUIM_NList_InsertSingle, tr(MSG_CO_SCRIPTS_NEWMSG), MUIV_NList_Insert_Bottom);
    break;

    case MACRO_READ:
      DoMethod(obj, MUIM_NList_InsertSingle, tr(MSG_CO_SCRIPTS_READ), MUIV_NList_Insert_Bottom);
    break;

    case MACRO_PREWRITE:
    case MACRO_POSTWRITE:
      DoMethod(obj, MUIM_NList_InsertSingle, tr(MSG_CO_SCRIPTS_WRITE), MUIV_NList_Insert_Bottom);
    break;

    case MACRO_URL:
      DoMethod(obj, MUIM_NList_InsertSingle, tr(MSG_CO_SCRIPTS_URL), MUIV_NList_Insert_Bottom);
    break;
  }

  LEAVE();
  return 0;
}

///
