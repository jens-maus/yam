/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2019 YAM Open Source Team

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
 Description: NList class of the read mail group's header list

***************************************************************************/

#include "HeaderList_cl.h"

#include <string.h>

#include <mui/NList_mcc.h>

#include "YAM_mainFolder.h"
#include "YAM_read.h"

#include "Config.h"
#include "Locale.h"
#include "MUIObjects.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  struct ReadMailData *rmData;
  char dateBuffer[64];
};
*/

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  ENTER();

  if((obj = DoSuperNew(cl, obj,

    InputListFrame,
    MUIA_NList_Format,               "P=\033r\0338 W=-1 MIW=-1,",
    MUIA_NList_Input,                FALSE,
    MUIA_NList_AutoClip,             C->AutoClip,
    MUIA_NList_AutoCopyToClip,       FALSE,
    MUIA_NList_TypeSelect,           MUIV_NList_TypeSelect_Char,
    MUIA_NList_ActiveObjectOnClick,  TRUE,
    MUIA_NList_DefaultObjectOnClick, FALSE,
    MUIA_ContextMenu,                FALSE,
    MUIA_CycleChain,                 TRUE,

    TAG_MORE, inittags(msg))) != NULL)
  {
    GETDATA;

    data->rmData = (struct ReadMailData *)GetTagData(ATTR(ReadMailData), (IPTR)NULL, inittags(msg));
  }

  RETURN((IPTR)obj);
  return (IPTR)obj;
}

///
/// OVERLOAD(MUIM_NList_Compare)
OVERLOAD(MUIM_NList_Compare)
{
  struct MUIP_NList_Compare *ncm = (struct MUIP_NList_Compare *)msg;
  struct HeaderNode *hdrNode1 = (struct HeaderNode *)ncm->entry1;
  struct HeaderNode *hdrNode2 = (struct HeaderNode *)ncm->entry2;
  GETDATA;
  LONG cmp;

  ENTER();

  // we sort the headerdisplay  not only by checking which
  // header should be displayed, but also by checking in
  // which order they should be displayed regarding the
  // specification in the short headers string object.
  if(data->rmData->headerMode == HM_SHORTHEADER && C->ShortHeaders[0] != '\0')
  {
    char *e1 = (hdrNode1->name != NULL) ? strcasestr(C->ShortHeaders, hdrNode1->name) : NULL;
    char *e2 = (hdrNode2->name != NULL) ? strcasestr(C->ShortHeaders, hdrNode2->name) : NULL;

    // now we compare the position of the found pointers
    // so that a lower pointer get higher priorities
    if(e1 != NULL && e2 != NULL)
      cmp = (LONG)(e1-e2);
    else if(e1 != NULL)
      cmp = -1;
    else if(e2 != NULL)
      cmp = +1;
    else
      cmp = 0;
  }
  else
    cmp = 0;

  RETURN(cmp);
  return cmp;
}

///
/// OVERLOAD(MUIM_NList_Display)
OVERLOAD(MUIM_NList_Display)
{
  struct MUIP_NList_Display *ndm = (struct MUIP_NList_Display *)msg;
  struct HeaderNode *hdrNode = (struct HeaderNode *)ndm->entry;
  GETDATA;

  ENTER();

  if(hdrNode != NULL)
  {
    // we translate some common header names into the local
    // language
    if(stricmp("From", hdrNode->name) == 0)
      ndm->strings[0] = (STRPTR)tr(MSG_RE_HDR_FROM);
    else if(stricmp("To", hdrNode->name) == 0)
      ndm->strings[0] = (STRPTR)tr(MSG_RE_HDR_TO);
    else if(stricmp("Reply-To", hdrNode->name) == 0)
      ndm->strings[0] = (STRPTR)tr(MSG_RE_HDR_REPLYTO);
    else if(stricmp("Date", hdrNode->name) == 0)
      ndm->strings[0] = (STRPTR)tr(MSG_RE_HDR_DATE);
    else if(stricmp("Subject", hdrNode->name) == 0)
      ndm->strings[0] = (STRPTR)tr(MSG_RE_HDR_SUBJECT);
    else
      ndm->strings[0] = hdrNode->name;

    // set the content of the header line
    // check for a valid mail pointer first, it might happen that this method is
    // called with a yet invalid pointer. Calculating the pointer to the date
    // below is working, but dereferencing it will cause a crash otherwise.
    if(data->rmData->mail != NULL && stricmp("Date", hdrNode->name) == 0 && data->rmData->headerMode == HM_SHORTHEADER)
    {
      // some special treatment of the Date: header line in "short" mode
      // always show the date/time converted to local time
      DateStamp2String(data->dateBuffer, sizeof(data->dateBuffer), &data->rmData->mail->Date, C->DSListFormat, TZC_UTC2LOCAL);
      ndm->strings[1] = data->dateBuffer;
    }
    else
      ndm->strings[1] = hdrNode->content;
  }

  RETURN(0);
  return 0;
}

///

/* Private Functions */

/* Public Methods */

