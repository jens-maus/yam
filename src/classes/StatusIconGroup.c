/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2007 by YAM Open Source Team

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
 Description: Provides some GUI elements for displaying the mail
              status icons

***************************************************************************/

#include "StatusIconGroup_cl.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *statusIcon[MAX_STATUSIMG];
};
*/

/* Hooks */
/// LayoutHook
HOOKPROTONH(LayoutFunc, ULONG, Object *obj, struct MUI_LayoutMsg *lm)
{
  ENTER();

  switch(lm->lm_Type)
  {
    case MUILM_MINMAX:
    {
      Object *cstate = (Object *)lm->lm_Children->mlh_Head;
      Object *child;
      LONG maxMinHeight = 0;
      LONG maxMinWidth = 0;

      // we iterate through all our children and see how large we are getting
      while((child = NextObject(&cstate)))
      {
        // we know our childs and that we only carry Bodychunk objects
        maxMinHeight = MAX(_minheight(child)+2, maxMinHeight);
        maxMinWidth += _minwidth(child)+1;
      }

      // then set our calculated values
      lm->lm_MinMax.MinWidth  = maxMinWidth;
      lm->lm_MinMax.MinHeight = maxMinHeight;
      lm->lm_MinMax.DefWidth  = maxMinWidth;
      lm->lm_MinMax.DefHeight = MUI_MAXMAX;
      lm->lm_MinMax.MaxWidth  = maxMinWidth;
      lm->lm_MinMax.MaxHeight = MUI_MAXMAX;

      RETURN(0);
      return 0;
    }
    break;

    case MUILM_LAYOUT:
    {
      Object *cstate = (Object *)lm->lm_Children->mlh_Head;
      Object *child;
      LONG left = 0;
      LONG top = ((lm->lm_Layout.Height-1)-_minheight(obj))/2;

      // Layout function. Here, we have to call MUI_Layout() for each
      // our children. MUI wants us to place them in a rectangle
      // defined by (0,0,lm->lm_Layout.Width-1,lm->lm_Layout.Height-1)
      // We are free to put the children anywhere in this rectangle.
      while((child = NextObject(&cstate)))
      {
        LONG mw = _minwidth(child);

        if(!MUI_Layout(child, left, top, _minwidth(child), _minheight(child), 0))
        {
          RETURN(FALSE);
          return FALSE;
        }

        left += mw+1;
      }

      RETURN(TRUE);
      return TRUE;
    }
    break;
  }

  RETURN(MUILM_UNKNOWN);
  return MUILM_UNKNOWN;
}
MakeStaticHook(LayoutHook, LayoutFunc);
///

/* Private Functions */

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  // *don't* add MAXBCSTATUSIMG as size since it would fill the missing
  // entries with NULL values...
  static const struct { int status; const char *const name; } icon[] = {
    { SICON_ID_UNREAD,   "status_unread" },
    { SICON_ID_OLD,      "status_old" },
    { SICON_ID_FORWARD,  "status_forward" },
    { SICON_ID_REPLY,    "status_reply" },
    { SICON_ID_WAITSEND, "status_waitsend" },
    { SICON_ID_ERROR,    "status_error" },
    { SICON_ID_HOLD,     "status_hold" },
    { SICON_ID_SENT,     "status_sent" },
    { SICON_ID_NEW,      "status_new" },
    { SICON_ID_DELETE,   "status_delete" },
    { SICON_ID_DOWNLOAD, "status_download" },
    { SICON_ID_GROUP,    "status_group" },
    { SICON_ID_URGENT,   "status_urgent" },
    { SICON_ID_ATTACH,   "status_attach" },
    { SICON_ID_REPORT,   "status_report" },
    { SICON_ID_CRYPT,    "status_crypt" },
    { SICON_ID_SIGNED,   "status_signed" },
    { SICON_ID_MARK,     "status_mark" },
    { SICON_ID_SPAM,     "status_spam" }
  };
  Object *statusIcon[MAX_STATUSIMG];
  unsigned int i;

  // make sure that all icons are listed!
  ASSERT(ARRAY_SIZE(icon) == MAX_STATUSIMG);

  // prepare the status icons for adding it later on to our statusGroup object
  for(i = 0; i < ARRAY_SIZE(icon); i++)
    statusIcon[icon[i].status] = MakeImageObject(icon[i].name);

  // should be a compile-time nop!
  for(i = ARRAY_SIZE(icon); i < MAX_STATUSIMG; i++)
    statusIcon[i] = NULL;

  obj = DoSuperNew(cl, obj,
    MUIA_Group_Horiz,       TRUE,
    MUIA_Group_LayoutHook,  &LayoutHook,
    MUIA_ContextMenu,        FALSE,
  TAG_MORE, inittags(msg));

  if(obj)
  {
    struct Data *data = (struct Data *)INST_DATA(cl, obj);
    if(!data)
      return 0;

    // copy data from temp object to its real place
    memcpy(&data->statusIcon[0], &statusIcon[0], sizeof(statusIcon));
  }

  return (ULONG)obj;
}
///
/// OVERLOAD(OM_DISPOSE)
OVERLOAD(OM_DISPOSE)
{
  GETDATA;
  struct List *childList = (struct List *)xget(obj, MUIA_Group_ChildList);
  int i;

  // clear all children of the statusGroup first
  DoMethod(obj, MUIM_Group_InitChange);

  // we first remove all childs from our statusGroup
  if(childList)
  {
    Object *cstate = (Object *)childList->lh_Head;
    Object *child;

    while((child = NextObject(&cstate)))
    {
      for(i=0; i < MAX_STATUSIMG; i++)
      {
        if(data->statusIcon[i] == child)
        {
          DoMethod(obj, OM_REMMEMBER, child);

          continue;
        }
      }
    }
  }

  DoMethod(obj, MUIM_Group_ExitChange);

  // now we can free all status icons
  for(i=0; i < MAX_STATUSIMG; i++)
  {
    if(data->statusIcon[i])
      MUI_DisposeObject(data->statusIcon[i]);

    data->statusIcon[i] = NULL;
  }

  return DoSuperMethodA(cl, obj, msg);
}
///

/* Public Methods */
/// DECLARE(Update)
DECLARE(Update) // struct Mail *mail
{
  GETDATA;
  struct Mail *mail = msg->mail;
  struct List *childList = (struct List *)xget(obj, MUIA_Group_ChildList);

  // update the statusgroup by removing/adding items accordingly
  DoMethod(obj, MUIM_Group_InitChange);

  // we first remove all childs from our statusGroup
  if(childList)
  {
    Object *cstate = (Object *)childList->lh_Head;
    Object *child;

    while((child = NextObject(&cstate)))
    {
      int i;

      for(i=0; i < MAX_STATUSIMG; i++)
      {
        if(data->statusIcon[i] == child)
        {
          DoMethod(obj, OM_REMMEMBER, child);

          continue;
        }
      }

    }

    // now we can add the status icons depending on the set status flags of
    // the mail
    if((hasStatusError(mail) || isPartialMail(mail)) && data->statusIcon[SICON_ID_ERROR])
      DoMethod(obj, OM_ADDMEMBER, data->statusIcon[SICON_ID_ERROR]);
    else if(hasStatusQueued(mail) && data->statusIcon[SICON_ID_WAITSEND])
      DoMethod(obj, OM_ADDMEMBER, data->statusIcon[SICON_ID_WAITSEND]);
    else if(hasStatusSent(mail) && data->statusIcon[SICON_ID_SENT])
      DoMethod(obj, OM_ADDMEMBER, data->statusIcon[SICON_ID_SENT]);
    else if(hasStatusRead(mail) && data->statusIcon[SICON_ID_OLD])
      DoMethod(obj, OM_ADDMEMBER, data->statusIcon[SICON_ID_OLD]);
    else if(data->statusIcon[SICON_ID_UNREAD])
      DoMethod(obj, OM_ADDMEMBER, data->statusIcon[SICON_ID_UNREAD]);

    // StatusGroup 1 (importance level)
    if(getImportanceLevel(mail) == IMP_HIGH && data->statusIcon[SICON_ID_URGENT])
      DoMethod(obj, OM_ADDMEMBER, data->statusIcon[SICON_ID_URGENT]);

    // StatusGroup 2 (signed/crypted status)
    if(isMP_CryptedMail(mail) && data->statusIcon[SICON_ID_CRYPT])
      DoMethod(obj, OM_ADDMEMBER, data->statusIcon[SICON_ID_CRYPT]);
    else if(isMP_SignedMail(mail) && data->statusIcon[SICON_ID_SIGNED])
      DoMethod(obj, OM_ADDMEMBER, data->statusIcon[SICON_ID_SIGNED]);

    // StatusGroup 3 (report mail info)
    if(isMP_ReportMail(mail) && data->statusIcon[SICON_ID_REPORT])
      DoMethod(obj, OM_ADDMEMBER, data->statusIcon[SICON_ID_REPORT]);

    // StatusGroup 4 (multipart info)
    if(isMP_MixedMail(mail) && data->statusIcon[SICON_ID_ATTACH])
      DoMethod(obj, OM_ADDMEMBER, data->statusIcon[SICON_ID_ATTACH]);

    // StatusGroup 5 (New/Hold info)
    if(hasStatusNew(mail) && data->statusIcon[SICON_ID_NEW])
      DoMethod(obj, OM_ADDMEMBER, data->statusIcon[SICON_ID_NEW]);
    else if(hasStatusHold(mail) && data->statusIcon[SICON_ID_HOLD])
      DoMethod(obj, OM_ADDMEMBER, data->statusIcon[SICON_ID_HOLD]);

    // StatusGroup 6 (marked flag)
    if(hasStatusMarked(mail) && data->statusIcon[SICON_ID_MARK])
      DoMethod(obj, OM_ADDMEMBER, data->statusIcon[SICON_ID_MARK]);

    // StatusGroup 7 (Replied status)
    if(hasStatusReplied(mail) && data->statusIcon[SICON_ID_REPLY])
      DoMethod(obj, OM_ADDMEMBER, data->statusIcon[SICON_ID_REPLY]);

    // StatusGroup 8 (Forwarded status)
    if(hasStatusForwarded(mail) && data->statusIcon[SICON_ID_FORWARD])
      DoMethod(obj, OM_ADDMEMBER, data->statusIcon[SICON_ID_FORWARD]);

    // StatusGroup 9(Spam status)
    if(hasStatusSpam(mail) && data->statusIcon[SICON_ID_SPAM])
      DoMethod(obj, OM_ADDMEMBER, data->statusIcon[SICON_ID_SPAM]);
  }

  // signal that we have added/modified the status Group successfully
  DoMethod(obj, MUIM_Group_ExitChange);

  return 0;
}
///
