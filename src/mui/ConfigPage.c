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
 YAM OpenSource project     : http://sourceforge.net/projects/yamos/

 $Id$

 Superclass:  MUIC_Group
 Description: Base class for pages in the config window

***************************************************************************/

#include "ConfigPage_cl.h"

#include <proto/muimaster.h>

#include "YAM.h"

#include "Locale.h"

#include "mui/ConfigPageList.h"
#include "mui/ImageArea.h"

#include "Debug.h"

/* INCLUDE
#include "YAM_config.h"
#include "Locale.h"
#include "MUIObjects.h"
#include "Themes.h"
*/

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  enum ConfigPage page;
  const char *imageID;
  ULONG image;
  const char *title;
  const char *summary;
  Object *contents;

  ENTER();

  page = (enum ConfigPage)GetTagData(ATTR(Page), (IPTR)cp_Max, inittags(msg));
  switch(page)
  {
    case cp_FirstSteps:   imageID = "config_firststep_big";  image = CI_FIRSTSTEPBIG;  title = tr(MSG_CO_FIRSTSTEPS_TITLE); summary = tr(MSG_CO_FIRSTSTEPS_SUMMARY); break;
    case cp_TCPIP:        imageID = "config_network_big";    image = CI_NETWORKBIG;    title = tr(MSG_CO_TCPIP_TITLE);      summary = tr(MSG_CO_TCPIP_SUMMARY);      break;
    case cp_Identities:   imageID = "config_identities_big"; image = CI_IDENTITIESBIG; title = tr(MSG_CO_IDENTITIES_TITLE); summary = tr(MSG_CO_IDENTITIES_SUMMARY); break;
    case cp_Spam:         imageID = "config_spam_big";       image = CI_SPAMBIG;       title = tr(MSG_CO_SPAMFILTER_TITLE); summary = tr(MSG_CO_SPAMFILTER_SUMMARY); break;
    case cp_Read:         imageID = "config_read_big";       image = CI_READBIG;       title = tr(MSG_CO_READ_TITLE);       summary = tr(MSG_CO_READ_SUMMARY);       break;
    case cp_Write:        imageID = "config_write_big";      image = CI_WRITEBIG;      title = tr(MSG_CO_WRITE_TITLE);      summary = tr(MSG_CO_WRITE_SUMMARY);      break;
    case cp_ReplyForward: imageID = "config_answer_big";     image = CI_ANSWERBIG;     title = tr(MSG_CO_REPLY_TITLE);      summary = tr(MSG_CO_REPLY_SUMMARY);      break;
	case cp_Signature:    imageID = "config_signature_big";  image = CI_SIGNATUREBIG;  title = tr(MSG_CO_SIGNATURE_TITLE);  summary = tr(MSG_CO_SIGNATURE_SUMMARY);  break;
	case cp_Security:     imageID = "config_security_big";   image = CI_SECURITYBIG;   title = tr(MSG_CO_SECURITY_TITLE);   summary = tr(MSG_CO_SECURITY_SUMMARY);   break;
	case cp_MIME:         imageID = "config_mime_big";       image = CI_MIMEBIG;       title = tr(MSG_CO_MIME_TITLE);       summary = tr(MSG_CO_MIME_SUMMARY);       break;
	case cp_StartupQuit:  imageID = "config_start_big";      image = CI_STARTBIG;      title = tr(MSG_CO_STARTUP_TITLE);    summary = tr(MSG_CO_STARTUP_SUMMARY);    break;
    case cp_AddressBook:  imageID = "config_abook_big";      image = CI_ABOOKBIG;      title = tr(MSG_CO_ABOOK_TITLE);      summary = tr(MSG_CO_ABOOK_SUMMARY);      break;
    case cp_Update:       imageID = "config_update_big";     image = CI_UPDATEBIG;     title = tr(MSG_CO_UPDATE_TITLE);     summary = tr(MSG_CO_UPDATE_SUMMARY);     break;
    default:              imageID = NULL;                    image = CI_MAX;           title = NULL;                        summary = NULL;                          break;
  }

  contents = (Object *)GetTagData(ATTR(Contents), (IPTR)NULL, inittags(msg));

  if((obj = DoSuperNew(cl, obj,
    MUIA_Group_Horiz, FALSE,
    Child, HGroup,
      Child, MakeImageObject(imageID, G->theme.configImages[image]),
      Child, VGroup,
        Child, TextObject,
          MUIA_Text_PreParse, "\033b",
          MUIA_Text_Contents, title,
          MUIA_Text_Copy,     FALSE,
        End,
        Child, TextObject,
          MUIA_Text_Contents, summary,
          MUIA_Text_Copy,     FALSE,
          MUIA_Font,          MUIV_Font_Tiny,
        End,
      End,
    End,
    Child, RectangleObject,
      MUIA_Rectangle_HBar, TRUE,
      MUIA_FixHeight,      4,
    End,
  TAG_MORE, inittags(msg))) != NULL)
  {
    if(GetTagData(ATTR(UseScrollgroup), TRUE, inittags(msg)) == TRUE)
    {
      // add a scrollgroup with the given contents as virtual group
      // plus a spacer object at the bottom
      DoMethod(obj, OM_ADDMEMBER, ScrollgroupObject,
        MUIA_Scrollgroup_FreeHoriz, FALSE,
        MUIA_Scrollgroup_AutoBars, TRUE,
        MUIA_Scrollgroup_Contents, VGroupV,
          Child, contents,
        End,
      End);
      if(GetTagData(ATTR(AddSpacer), FALSE, inittags(msg)) == TRUE)
        DoMethod(contents, OM_ADDMEMBER, HVSpace);
    }
    else
    {
      // just add the contents
      DoMethod(obj, OM_ADDMEMBER, contents);
    }
  }

  RETURN((IPTR)obj);
  return (IPTR)obj;
}

///
/// OVERLOAD(OM_GET)
OVERLOAD(OM_GET)
{
  IPTR *store = ((struct opGet *)msg)->opg_Storage;
  ULONG result = FALSE;

  ENTER();

  switch(((struct opGet *)msg)->opg_AttrID)
  {
    case ATTR(ConfigUpdate): *store = TRUE; result = TRUE; break;
  }

  if(result == FALSE)
    result = DoSuperMethodA(cl, obj, msg);

  RETURN(result);
  return result;
}

///
/// DECLARE(ConfigToGUI)
DECLARE(ConfigToGUI)
{
  ENTER();

  E(DBF_GUI, "derived class did not overload MUIM_ConfigPage_ConfigToGUI method");

  RETURN(0);
  return 0;
}

///
/// DECLARE(GUIToConfig)
DECLARE(GUIToConfig)
{
  ENTER();

  E(DBF_GUI, "derived class did not overload MUIM_ConfigPage_GUIToConfig method");

  RETURN(0);
  return 0;
}

///
/// DECLARE(ConfigUpdate)
DECLARE(ConfigUpdate) // enum ConfigPage sourcePage
{
  ENTER();

  // this method is optional to be overloaded

  RETURN(0);
  return 0;
}

///
