/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2025 YAM Open Source Team

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
 YAM OpenSource project    :  https://github.com/jens-maus/yam/

 Superclass:  MUIC_NList
 Description: NList class to display variable placeholders

***************************************************************************/

#include "PlaceholderPopupList_cl.h"

#include <string.h>
#include <mui/NList_mcc.h>

#include "YAM_utilities.h"

#include "Locale.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  const char *entries[16];
  char placeholder[8];
  char description[SIZE_LARGE];
};
*/

/* EXPORT
enum PlaceholderMode
{
  PHM_FORWARD=0,
  PHM_REPLYHELLO,
  PHM_REPLYINTRO,
  PHM_REPLYBYE,
  PHM_ARCHIVE,
  PHM_MAILSTATS,
  PHM_SCRIPTS,
  PHM_MIME_DEFVIEWER,
  PHM_MIME_COMMAND,
};
*/

/* INCLUDE
#include "YAM_main.h" // for enum Macro
*/

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  enum PlaceholderMode mode;

  ENTER();

  mode = (enum PlaceholderMode)GetTagData(ATTR(Mode), PHM_FORWARD, inittags(msg));

  if((obj = DoSuperNew(cl, obj,

    InputListFrame,
    MUIA_NList_AdjustHeight, TRUE,
    MUIA_NList_Format, ",",

    TAG_MORE, inittags(msg))) != NULL)
  {
    GETDATA;

    switch(mode)
    {
      case PHM_FORWARD:
      case PHM_REPLYHELLO:
      case PHM_REPLYINTRO:
      case PHM_REPLYBYE:
      {
        data->entries[ 0] = tr(MSG_CO_LineBreak);
        data->entries[ 1] = (mode == PHM_FORWARD) ? tr(MSG_CO_ORecptName) : tr(MSG_CO_RecptName);
        data->entries[ 2] = (mode == PHM_FORWARD) ? tr(MSG_CO_ORecptFirstname) : tr(MSG_CO_RecptFirstname);
        data->entries[ 3] = (mode == PHM_FORWARD) ? tr(MSG_CO_ORecptAddress) : tr(MSG_CO_RecptAddress);
        data->entries[ 4] = tr(MSG_CO_SenderName);
        data->entries[ 5] = tr(MSG_CO_SenderFirstname);
        data->entries[ 6] = tr(MSG_CO_SenderAddress);
        data->entries[ 7] = tr(MSG_CO_SenderSubject);
        data->entries[ 8] = tr(MSG_CO_SenderRFCDateTime);
        data->entries[ 9] = tr(MSG_CO_SenderDate);
        data->entries[10] = tr(MSG_CO_SenderTime);
        data->entries[11] = tr(MSG_CO_SenderTimeZone);
        data->entries[12] = tr(MSG_CO_SenderDOW);
        data->entries[13] = tr(MSG_CO_SenderMsgID);
        // depending on the mode we have the "CompleteHeader" feature or not
        data->entries[14] = (mode == PHM_FORWARD || mode == PHM_REPLYINTRO) ? tr(MSG_CO_CompleteHeader) : NULL;
        data->entries[15] = NULL;
      }
      break;

      case PHM_ARCHIVE:
      {
        data->entries[0] = tr(MSG_CO_ArchiveName);
        data->entries[1] = tr(MSG_CO_ArchiveFiles);
        data->entries[2] = tr(MSG_CO_ArchiveFilelist);
        data->entries[3] = NULL;
      }
      break;

      case PHM_MAILSTATS:
      {
        data->entries[0] = tr(MSG_CO_NEWMSGS);
        data->entries[1] = tr(MSG_CO_UNREADMSGS);
        data->entries[2] = tr(MSG_CO_TOTALMSGS);
        data->entries[3] = tr(MSG_CO_SENTMSGS);
        data->entries[4] = NULL;
      }
      break;

      case PHM_SCRIPTS:
      {
        // nothing to insert here, this is done externally depending on the type of the script
        data->entries[0] = NULL;
      }
      break;

      case PHM_MIME_DEFVIEWER:
      case PHM_MIME_COMMAND:
      {
        data->entries[0] = tr(MSG_CO_MIMECMD_PARAMETER);
        data->entries[1] = tr(MSG_CO_MIMECMD_PUBSCREEN);
        data->entries[2] = NULL;
      }
      break;
    }

    if(data->entries[0] != NULL)
      DoMethod(obj, MUIM_NList_Insert, data->entries, -1, MUIV_NList_Insert_Bottom, MUIF_NONE);
  }

  RETURN((IPTR)obj);
  return (IPTR)obj;
}

///
/// OVERLOAD(OM_SET)
OVERLOAD(OM_SET)
{
  GETDATA;
  struct TagItem *tags = inittags(msg), *tag;
  ULONG result;

  ENTER();

  while((tag = NextTagItem((APTR)&tags)) != NULL)
  {
    switch(tag->ti_Tag)
    {
      case ATTR(ScriptEntry):
      {
        // clear the list first
        DoMethod(obj, MUIM_NList_Clear);

        switch((enum Macro)tag->ti_Data)
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
            data->entries[0] = NULL;
          break;

          case MACRO_PREGET:
            data->entries[0] = tr(MSG_CO_SCRIPTS_PREGET);
          break;

          case MACRO_POSTGET:
            data->entries[0] = tr(MSG_CO_SCRIPTS_POSTGET);
          break;

          case MACRO_NEWMSG:
            data->entries[0] = tr(MSG_CO_SCRIPTS_NEWMSG);
          break;

          case MACRO_READ:
            data->entries[0] = tr(MSG_CO_SCRIPTS_READ);
          break;

          case MACRO_PREWRITE:
          case MACRO_POSTWRITE:
            data->entries[0] = tr(MSG_CO_SCRIPTS_WRITE);
          break;

          case MACRO_URL:
            data->entries[0] = tr(MSG_CO_SCRIPTS_URL);
          break;
        }

        data->entries[1] = NULL;

        if(data->entries[0] != NULL)
          DoMethod(obj, MUIM_NList_Insert, data->entries, 1, MUIV_NList_Insert_Bottom, MUIF_NONE);

        tag->ti_Tag = TAG_IGNORE;
      }
      break;
    }
  }

  result = DoSuperMethodA(cl, obj, msg);

  RETURN(result);
  return result;
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

  RETURN(0);
  return 0;
}

///
