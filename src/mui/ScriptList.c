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
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.   See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 YAM Official Support Site : http://www.yam.ch
 YAM OpenSource project    : http://sourceforge.net/projects/yamos/

 $Id$

 Superclass:  MUIC_NList
 Description: a list showing all available scripts

***************************************************************************/

#include "ScriptList_cl.h"

#include <string.h>

#include <mui/NList_mcc.h>

#include "Config.h"
#include "Locale.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  char title[SIZE_DEFAULT];
};
*/

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  ENTER();

  obj = DoSuperNew(cl, obj,

    InputListFrame,
    MUIA_NList_Title,        TRUE,
    MUIA_NList_Format,       "BAR,",

    TAG_MORE, inittags(msg));

  RETURN((IPTR)obj);
  return (IPTR)obj;
}

///
/// OVERLOAD(MUIM_NList_Display)
OVERLOAD(MUIM_NList_Display)
{
  struct MUIP_NList_Display *ndm = (struct MUIP_NList_Display *)msg;

  ENTER();

  if(CE != NULL)
  {
    if(ndm->entry != NULL)
    {
      GETDATA;
      enum Macro type = (enum Macro)(ndm->entry)-1;

      data->title[0] = '\0';

      switch(type)
      {
        case MACRO_STARTUP:    strlcpy(data->title, tr(MSG_CO_ScriptStartup), sizeof(data->title)); break;
        case MACRO_QUIT:       strlcpy(data->title, tr(MSG_CO_ScriptTerminate), sizeof(data->title)); break;
        case MACRO_PREGET:     strlcpy(data->title, tr(MSG_CO_ScriptPreGetMail), sizeof(data->title)); break;
        case MACRO_POSTGET:    strlcpy(data->title, tr(MSG_CO_ScriptPostGetMail), sizeof(data->title)); break;
        case MACRO_NEWMSG:     strlcpy(data->title, tr(MSG_CO_ScriptNewMsg), sizeof(data->title)); break;
        case MACRO_PRESEND:    strlcpy(data->title, tr(MSG_CO_ScriptPreSendMail), sizeof(data->title)); break;
        case MACRO_POSTSEND:   strlcpy(data->title, tr(MSG_CO_ScriptPostSendMail), sizeof(data->title)); break;
        case MACRO_READ:       strlcpy(data->title, tr(MSG_CO_ScriptReadMsg), sizeof(data->title)); break;
        case MACRO_PREWRITE:   strlcpy(data->title, tr(MSG_CO_ScriptPreWriteMsg), sizeof(data->title)); break;
        case MACRO_POSTWRITE:  strlcpy(data->title, tr(MSG_CO_ScriptPostWriteMsg), sizeof(data->title)); break;
        case MACRO_URL:        strlcpy(data->title, tr(MSG_CO_ScriptClickURL), sizeof(data->title)); break;
        case MACRO_PREFILTER:  strlcpy(data->title, tr(MSG_CO_ScriptPreFilterMail), sizeof(data->title)); break;
        case MACRO_POSTFILTER: strlcpy(data->title, tr(MSG_CO_ScriptPostFilterMail), sizeof(data->title)); break;

        // the user definable macros
        default:
        {
          snprintf(data->title, sizeof(data->title), tr(MSG_CO_ScriptMenu), type+1);

          if(CE->RX[type].Name[0] != '\0')
            snprintf(data->title, sizeof(data->title), "%s (%s)", data->title, CE->RX[type].Name);
        }
      }

      ndm->strings[0] = data->title;

      if(CE->RX[type].Script[0] != '\0')
      {
        ndm->strings[1] = CE->RX[type].Script;
        ndm->preparses[0] = (char *)MUIX_B;
      }
    }
    else
    {
      ndm->strings[0] = (char *)tr(MSG_CO_SCRIPTACTION);
      ndm->strings[1] = (char *)tr(MSG_CO_SCRIPTPATH);
    }
  }
  else
  {
    ndm->strings[0] = NULL;
    ndm->strings[1] = NULL;
  }

  RETURN(0);
  return 0;
}

///
