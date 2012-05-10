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
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.   See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 YAM Official Support Site : http://www.yam.ch
 YAM OpenSource project    : http://sourceforge.net/projects/yamos/

 $Id$

 Superclass:  MUIC_BetterString
 Description: Auto-completes email addresses etc.

***************************************************************************/

#include "Recipientstring_cl.h"

#include <ctype.h>
#include <string.h>
#include <proto/muimaster.h>
#include <mui/BetterString_mcc.h>
#include <mui/NList_mcc.h>

#include "SDI_hook.h"
#include "newmouse.h"

#include "YAM.h"
#include "YAM_addressbook.h"
#include "YAM_addressbookEntry.h"
#include "YAM_config.h"
#include "YAM_mainFolder.h"
#include "MUIObjects.h"
#include "UserIdentity.h"

#include "mui/AddressmatchPopup.h"
#include "mui/MainMailListGroup.h"
#include "mui/YAMApplication.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  struct MUI_EventHandlerNode ehnode;
  Object *Matchwindow;
  Object *ReplyTo;                    // used when resolving a list address
  STRPTR CurrentRecipient;
  ULONG bufferPos;
  ULONG selectSize;
  BOOL isActive;
  BOOL MultipleRecipients;
  BOOL ResolveOnCR;
  BOOL AdvanceOnCR;                   // we have to save this attribute ourself because Betterstring.mcc is buggy.
  BOOL NoFullName;
  BOOL NoCache;
  BOOL NoValid;
  BOOL ResolveOnInactive;
  struct UserIdentityNode *identity;  // ptr to the user identity currently active (or NULL)
};
*/

/* EXPORT
#define MUIF_Recipientstring_Resolve_NoFullName  (1 << 0) // do not resolve with fullname "Mister X <misterx@mister.com>"
#define MUIF_Recipientstring_Resolve_NoValid     (1 << 1) // do not resolve already valid string like "misterx@mister.com"
#define MUIF_Recipientstring_Resolve_NoCache     (1 << 2) // do not resolve addresses out of the eMailCache

#define hasNoFullNameFlag(v)   (isFlagSet((v), MUIF_Recipientstring_Resolve_NoFullName))
#define hasNoValidFlag(v)      (isFlagSet((v), MUIF_Recipientstring_Resolve_NoValid))
#define hasNoCacheFlag(v)      (isFlagSet((v), MUIF_Recipientstring_Resolve_NoCache))
*/

/* Hooks */
/// FindAddressHook
HOOKPROTONHNO(FindAddressFunc, LONG, struct MUIP_NListtree_FindUserDataMessage *msg)
{
  struct ABEntry *entry;
  LONG result = ~0;

  ENTER();

  entry = (struct ABEntry *)msg->UserData;

  if(entry->Type == AET_USER || entry->Type == AET_LIST)
  {
    // first see if our string matches the alias
    if(Stricmp(msg->User, entry->Alias) == 0)
    {
      D(DBF_GUI, "\"%s\" matches alias \"%s\"", msg->User, entry->Alias);
      result = 0;
    }
    else
    {
      // then we search for the realname
      char *unquoted = UnquoteString(entry->RealName, TRUE);

      if(Stricmp(msg->User, unquoted) == 0)
      {
        D(DBF_GUI, "\"%s\" matches realname \"%s\"", msg->User, entry->RealName);
        result = 0;
      }
      else if(entry->Type == AET_USER && Stricmp(msg->User, entry->Address) == 0)
      {
        D(DBF_GUI, "\"%s\" matches address \"%s\"", msg->User, entry->Address);
        result = 0;
      }

      free(unquoted);
    }
  }

  RETURN(result);
  return result;
}
MakeStaticHook(FindAddressHook, FindAddressFunc);
///

/* Private Functions */
/// rcptok()
// Non-threadsafe strtok() alike recipient tokenizer.
// "," is the hardcoded token. Ignored if surrounded by quotes ("").
static char *rcptok(char *s, BOOL *quote)
{
  static char *p;

  ENTER();

  if(s != NULL)
    p = s;
  else
    s = p;

  if(p == NULL || *p == '\0')
  {
    RETURN(NULL);
    return NULL;
  }

  while(*p != '\0')
  {
    if(*p == '"')
    {
      *quote = !(*quote);
    }
    else if(*p == ',' && *quote == FALSE)
    {
      *p++ = '\0';
	    break;
    }

    p++;
  }

  // strip leading and trailing blanks
  s = Trim(s);

  RETURN(s);
  return s;
}
///
/// NormalizeSelection()
// normalized/clears the selection and removes eventually existing
// ' >> ' marks.
static void NormalizeSelection(Object *obj, BOOL clear)
{
  LONG start = DoMethod(obj, METHOD(RecipientStart));
  LONG rcpSize;
  LONG marksSize = 0;
  char *rcp = (char *)xget(obj, MUIA_String_Contents) + start;
  char *p;

  ENTER();

  // Skip the " >> " marks if they have been inserted before.
  // Also remember how many chars have been skipped, as the
  // complete current recipient will be replaced later.
  if((p = strstr(rcp, " >> ")) != NULL)
  {
    marksSize = (LONG)(p + 4 - rcp);
    rcp = strdup(p + 4);
  }
  else if(clear)
    rcp = strdup(rcp);
  else
    rcp = NULL;

  if(rcp != NULL)
  {
    // calculate the length of the current recipient
    if((p = strchr(rcp, ',')) != NULL)
      rcpSize = (LONG)(p - rcp);
    else
      rcpSize = strlen(rcp);

    // NUL-terminate this recipient
    rcp[rcpSize] = '\0';

    // stop betterstring from sending out notifications
    set(obj, MUIA_BetterString_NoNotify, TRUE);

    // remove the current recipient from the string, including the " >> " marks
    // and everything typed so far
    xset(obj, MUIA_String_BufferPos, start,
              MUIA_BetterString_SelectSize, rcpSize + marksSize);

    DoMethod(obj, MUIM_BetterString_ClearSelected);

    // now insert the correct recipient again
    DoMethod(obj, MUIM_BetterString_Insert, rcp, start);

    // turn notifications back on and trigger one
    // immediately since Insert() was called.
    set(obj, MUIA_BetterString_NoNotify, FALSE);

    free(rcp);
  }

  LEAVE();
}

///
/// InsertAddress
//  Adds a new recipient to a recipient field
static void InsertAddress(Object *obj, const char *alias, const char *name, const char *address)
{
  ENTER();

  if(*alias != '\0')
  {
    // If we have an alias we just add this one and let the string object
    // resolve this alias later.
    DoMethod(obj, METHOD(AddRecipient), alias);
  }
  else
  {
    // Otherwise we build a string like "name <address>" based on the given
    // informations. Names containing commas will be quoted.
    char fullName[SIZE_REALNAME+SIZE_ADDRESS];

    if(strchr(name, ',') != NULL)
      snprintf(fullName, sizeof(fullName), "\"%s\"", name);
    else
      strlcpy(fullName, name, sizeof(fullName));

    if(*address != '\0')
    {
      if(fullName[0] != '\0')
        strlcat(fullName, " <", sizeof(fullName));
      strlcat(fullName, address, sizeof(fullName));
      if(fullName[0] != '\0')
        strlcat(fullName, ">", sizeof(fullName));
    }

    DoMethod(obj, METHOD(AddRecipient), fullName);
  }
}

///


/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  ENTER();

  if((obj = DoSuperNew(cl, obj,
    StringFrame,
    MUIA_BetterString_NoShortcuts, TRUE,
    TAG_MORE, inittags(msg))) != NULL)
  {
    GETDATA;
    struct TagItem *tags = inittags(msg), *tag;

    while((tag = NextTagItem((APTR)&tags)) != NULL)
    {
      switch(tag->ti_Tag)
      {
        case ATTR(ResolveOnCR)        : data->ResolveOnCR = tag->ti_Data        ; break;
        case ATTR(MultipleRecipients) : data->MultipleRecipients = tag->ti_Data ; break;
        case ATTR(ReplyToString)      : data->ReplyTo = (Object *)tag->ti_Data  ; break;
        case ATTR(NoFullName)         : data->NoFullName = tag->ti_Data         ; break;
        case ATTR(NoCache)            : data->NoCache = tag->ti_Data            ; break;
        case ATTR(NoValid)            : data->NoValid = tag->ti_Data            ; break;
        case ATTR(ResolveOnInactive)  : data->ResolveOnInactive = tag->ti_Data  ; break;

        // we also catch foreign attributes
        case MUIA_String_AdvanceOnCR: data->AdvanceOnCR = tag->ti_Data     ; break;
      }
    }

    xset(obj, MUIA_String_Popup, obj,
              MUIA_String_Reject, data->MultipleRecipients ? NULL : ",");
  }

  RETURN((IPTR)obj);
  return (IPTR)obj;
}
///
/// OVERLOAD(OM_DISPOSE)
OVERLOAD(OM_DISPOSE)
{
  GETDATA;

  if(data->Matchwindow != NULL)
  {
    D(DBF_GUI, "Dispose addrlistpopup: %08lx", data->Matchwindow);

    // we know that _app(obj) is documented as only valid in
    // MUIM_Cleanup/Setup, but these two methods are also called
    // if a window gets iconfied only. And by looking at the MUI source,
    // MUI first calls each OM_DISPOSE of all children, so _app(obj)
    // should also be valid during a OM_DISPOSE call, and obviously it
    // doesn`t cause any problem.
    DoMethod(_app(obj), OM_REMMEMBER, data->Matchwindow);
    MUI_DisposeObject(data->Matchwindow);

    data->Matchwindow = NULL;
  }

  free(data->CurrentRecipient);
  data->CurrentRecipient = NULL;

  return DoSuperMethodA(cl, obj, msg);
}
///
/// OVERLOAD(OM_GET)
/* this is just so that we can notify the popup tag */
OVERLOAD(OM_GET)
{
  GETDATA;
  IPTR *store = ((struct opGet *)msg)->opg_Storage;

  switch(((struct opGet *)msg)->opg_AttrID)
  {
    case ATTR(Popup): *store = FALSE ; return TRUE;
    case ATTR(NoFullName): *store = data->NoFullName; return TRUE;

    // we also return foreign attributes
    case MUIA_String_AdvanceOnCR: *store = data->AdvanceOnCR; return TRUE;
  }

  return DoSuperMethodA(cl, obj, msg);
}
///
/// OVERLOAD(OM_SET)
OVERLOAD(OM_SET)
{
  GETDATA;
  struct TagItem *tags = inittags(msg), *tag;

  while((tag = NextTagItem((APTR)&tags)) != NULL)
  {
    switch(tag->ti_Tag)
    {
      case ATTR(ResolveOnCR):
      {
        data->ResolveOnCR = tag->ti_Data;

        // make the superMethod call ignore those tags
        tag->ti_Tag = TAG_IGNORE;
      }
      break;

      case ATTR(MultipleRecipients):
      {
        data->MultipleRecipients = tag->ti_Data;

        // make the superMethod call ignore those tags
        tag->ti_Tag = TAG_IGNORE;
      }
      break;

      case ATTR(NoFullName):
      {
        data->NoFullName = tag->ti_Data;

        // make the superMethod call ignore those tags
        tag->ti_Tag = TAG_IGNORE;
      }
      break;

      case ATTR(NoCache):
      {
        data->NoCache = tag->ti_Data;

        // make the superMethod call ignore those tags
        tag->ti_Tag = TAG_IGNORE;
      }
      break;

      case ATTR(NoValid):
      {
        data->NoValid = tag->ti_Data;

        // make the superMethod call ignore those tags
        tag->ti_Tag = TAG_IGNORE;
      }
      break;

      case ATTR(ResolveOnInactive):
      {
        data->ResolveOnInactive = tag->ti_Data;

        // make the superMethod call ignore those tags
        tag->ti_Tag = TAG_IGNORE;
      }
      break;

      case ATTR(ReplyToString):
      {
        data->ReplyTo = (Object *)tag->ti_Data;

        // make the superMethod call ignore those tags
        tag->ti_Tag = TAG_IGNORE;
      }
      break;

      case ATTR(ActiveIdentity):
      {
        data->identity = (struct UserIdentityNode *)tag->ti_Data;

        // make the superMethod call ignore those tags
        tag->ti_Tag = TAG_IGNORE;
      }
      break;

      // we also catch foreign attributes
      case MUIA_String_AdvanceOnCR:
      {
        data->AdvanceOnCR = tag->ti_Data;
      }
      break;
    }
  }

  return DoSuperMethodA(cl, obj, msg);
}
///
/// OVERLOAD(MUIM_Setup)
OVERLOAD(MUIM_Setup)
{
  GETDATA;
  IPTR result;

  ENTER();

  // create the address match list object, if it does not exist yet
  if(data->Matchwindow == NULL)
  {
    data->Matchwindow = AddressmatchPopupObject,
      MUIA_AddressmatchPopup_String, obj,
    End;
    D(DBF_GUI, "Create addrlistpopup: %08lx", data->Matchwindow);
  }

  if(data->Matchwindow != NULL)
  {
    if((result = DoSuperMethodA(cl, obj, msg)))
    {
      data->ehnode.ehn_Priority = 1;
      data->ehnode.ehn_Flags    = 0;
      data->ehnode.ehn_Object   = obj;
      data->ehnode.ehn_Class    = cl;

      #if defined(__amigaos4__)
      data->ehnode.ehn_Events   = IDCMP_RAWKEY | IDCMP_CHANGEWINDOW | IDCMP_EXTENDEDMOUSE;
      #else
      data->ehnode.ehn_Events   = IDCMP_RAWKEY | IDCMP_CHANGEWINDOW;
      #endif
    }
  }
  else
    result = FALSE;

  RETURN(result);
  return result;
}
///
/// OVERLOAD(MUIM_Show)
OVERLOAD(MUIM_Show)
{
  GETDATA;
  IPTR result;

  ENTER();

  DoMethod(data->Matchwindow, MUIM_AddressmatchPopup_ChangeWindow);
  result = DoSuperMethodA(cl, obj, msg);

  RETURN(result);
  return result;
}
///
/// OVERLOAD(MUIM_GoActive)
OVERLOAD(MUIM_GoActive)
{
  GETDATA;
  IPTR result;

  ENTER();

  // remember the active state
  data->isActive = TRUE;

  // install the event handler so we listen for any key/mouse
  // events we configured in MUIM_Setup()
  DoMethod(_win(obj), MUIM_Window_AddEventHandler, &data->ehnode);

  // call the super method
  result = DoSuperMethodA(cl, obj, msg);

  RETURN(result);
  return result;
}
///
/// OVERLOAD(MUIM_GoInactive)
OVERLOAD(MUIM_GoInactive)
{
  GETDATA;
  IPTR result;

  ENTER();

  // remove the event handler so we don't listen for any further
  // key/mouse events
  DoMethod(_win(obj), MUIM_Window_RemEventHandler, &data->ehnode);

  // call NormalizeSelection() to make sure that
  // no ' >> ' marks are kept when the gadget is going
  // into inactive state.
  NormalizeSelection(obj, TRUE);

  // only if the matchwindow is not active we can close it on a inactive state of
  // this object
  if(xget(data->Matchwindow, MUIA_Window_Activate) == FALSE)
    set(data->Matchwindow, MUIA_Window_Open, FALSE);

  // we check if the user wants us to resolve the content
  // of the string also when the string gadgets goes inactive.
  if(data->ResolveOnInactive == TRUE)
    DoMethod(obj, METHOD(Resolve), MUIF_NONE);

  // remember the current selection before our superclass deletes these values because of going inactive
  data->bufferPos = xget(obj, MUIA_String_BufferPos);
  data->selectSize = xget(obj, MUIA_BetterString_SelectSize);

  // remember the inactive state
  data->isActive = FALSE;

  // call the super method
  result = DoSuperMethodA(cl, obj, msg);

  RETURN(result);
  return result;
}
///
/// OVERLOAD(MUIM_DragQuery)
OVERLOAD(MUIM_DragQuery)
{
  struct MUIP_DragQuery *d = (struct MUIP_DragQuery *)msg;
  IPTR result = MUIV_DragQuery_Refuse;

  ENTER();

  if(d->obj == G->AB->GUI.LV_ADDRESSES)
  {
    struct MUI_NListtree_TreeNode *active;

    if((active = (struct MUI_NListtree_TreeNode *)xget(d->obj, MUIA_NListtree_Active)) != NULL)
    {
      if(isFlagClear(active->tn_Flags, TNF_LIST))
        result = MUIV_DragQuery_Accept;
    }
  }
  else if(DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_MainMailListGroup_IsMailList, d->obj) == TRUE)
  {
    result = MUIV_DragQuery_Accept;
  }

  RETURN(result);
  return result;
}
///
/// OVERLOAD(MUIM_DragDrop)
OVERLOAD(MUIM_DragDrop)
{
  struct MUIP_DragQuery *d = (struct MUIP_DragQuery *)msg;

  ENTER();

  if(d->obj == G->AB->GUI.LV_ADDRESSES)
  {
    struct MUI_NListtree_TreeNode *active = (struct MUI_NListtree_TreeNode *)xget(d->obj, MUIA_NListtree_Active);
    struct ABEntry *addr = (struct ABEntry *)(active->tn_User);

    InsertAddress(obj, addr->Alias, addr->RealName, "");
  }
  else if(DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_MainMailListGroup_IsMailList, d->obj) == TRUE)
  {
    struct Mail *mail;

    DoMethod(d->obj, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &mail);

    if(isSentMailFolder(mail->Folder))
      InsertAddress(obj, "", mail->To.RealName,   mail->To.Address);
    else
      InsertAddress(obj, "", mail->From.RealName, mail->From.Address);
  }

  RETURN(0);
  return 0;
}
///
/// OVERLOAD(MUIM_Popstring_Open)
/* this method is invoked when the MUI popup key is pressed, we let it trigger a notify, so that the address book will open -- in the future this should be removed and we should just use a Popupstring object */
OVERLOAD(MUIM_Popstring_Open)
{
  ENTER();

  set(obj, MUIA_Recipientstring_Popup, TRUE);

  RETURN(0);
  return 0;
}
///
/// OVERLOAD(MUIM_HandleEvent)
OVERLOAD(MUIM_HandleEvent)
{
  GETDATA;
  IPTR result = 0;
  struct IntuiMessage *imsg;

  ENTER();

  if((imsg = ((struct MUIP_HandleEvent *)msg)->imsg) != NULL)
  {
    D(DBF_GUI, "event muikey %ld code %02lx", ((struct MUIP_HandleEvent *)msg)->muikey, ((struct MUIP_HandleEvent *)msg)->imsg->Code);

    if(imsg->Class == IDCMP_RAWKEY)
    {
      switch(imsg->Code)
      {
        case IECODE_RETURN:
        {
          if(data->ResolveOnCR == TRUE)
          {
            // only if we successfully resolved the string we move on to the next object.
            if(DoMethod(obj, METHOD(Resolve), hasFlag(imsg->Qualifier, (IEQUALIFIER_RSHIFT | IEQUALIFIER_LSHIFT)) ? MUIF_Recipientstring_Resolve_NoFullName : MUIF_NONE))
            {
              BOOL matchListWasOpen = xget(data->Matchwindow, MUIA_Window_Open);

              set(data->Matchwindow, MUIA_Window_Open, FALSE);
              set(_win(obj), MUIA_Window_ActiveObject, obj);

              // If the MUIA_String_AdvanceOnCR is TRUE we have to set the next object active in the window
              // we have to check this within our instance data because Betterstring.mcc is buggy and doesn't
              // return MUIA_String_AdvanceOnCR within a get().
              // We skip the advance to the next object if the matchlist was open, because
              // if the user pressed Enter/Return with an open matchlist (s)he usually just
              // wanted to add the selected recipient instead of advancing to the subject
              // line.
              if(data->AdvanceOnCR == TRUE &&
                 (matchListWasOpen == FALSE || data->MultipleRecipients == FALSE))
              {
                set(_win(obj), MUIA_Window_ActiveObject, MUIV_Window_ActiveObject_Next);
              }
              else
              {
                // add a separator as more recipients may be wanted
                DoMethod(obj, MUIM_BetterString_Insert, ", ", MUIV_BetterString_Insert_EndOfString);
              }
            }
            else
              DisplayBeep(_screen(obj));

            result = MUI_EventHandlerRC_Eat;
          }
        }
        break;

        // keys that are sent to the popup-list
        case IECODE_UP:
        case IECODE_DOWN:
        case NM_WHEEL_UP:
        case NM_WHEEL_DOWN:
        case NM_WHEEL_LEFT:
        case NM_WHEEL_RIGHT:
        {
          // forward this event to the addrmatchlist
          if(DoMethod(data->Matchwindow, MUIM_AddressmatchPopup_Event, imsg))
            result = MUI_EventHandlerRC_Eat;
        }
        break;

        // keys that clear the marked area
        // and close the popup-list
        case IECODE_DEL:
        case IECODE_ESCAPE:
        {
          DoMethod(obj, MUIM_BetterString_ClearSelected);
          set(data->Matchwindow, MUIA_Window_Open, FALSE);

          // Escape should clear the marked text, but somehow it isn't
          // done automatomatically which seems to be a refresh problem.
          // Therefore we force a refresh here.
          if(imsg->Code == IECODE_ESCAPE)
            MUI_Redraw(obj, MADF_DRAWOBJECT);
        }
        break;

        // IECODE_TAB will only be triggered if the tab key
        // is hit and the matchwindow is currently open. This is more
        // or less a workaround for a problem where the addrmatchlist
        // window can get the key focus - somehow...
        case IECODE_TAB:
        {
          if(xget(data->Matchwindow, MUIA_Window_Open) == TRUE)
          {
            set(data->Matchwindow, MUIA_Window_Open, FALSE);
            set(_win(obj), MUIA_Window_ActiveObject, obj);
            set(_win(obj), MUIA_Window_ActiveObject, MUIV_Window_ActiveObject_Next);
          }
        }
        break;

        default:
        {
          BOOL closeMatchWin = FALSE;
          LONG selectSize;

          // check if some text is actually selected
          if((selectSize = xget(obj, MUIA_BetterString_SelectSize)) != 0)
          {
            if(imsg->Code == IECODE_BACKSPACE)
            {
              // now we do check whether everything until the end was selected
              LONG pos = xget(obj, MUIA_String_BufferPos) + selectSize;
              char *content = (char *)xget(obj, MUIA_String_Contents);

              if(content != NULL && (content[pos] == '\0' || content[pos] == ','))
                DoMethod(obj, MUIM_BetterString_ClearSelected);
            }
            else if((imsg->Code == IECODE_LEFT || imsg->Code == IECODE_RIGHT))
            {
              // call NormalizeSelection() to make sure that
              // no ' >> ' marks are kept when the user wants to continue
              // with either the next/prev or by simply pressing left/right
              // to finish the selection.
              NormalizeSelection(obj, FALSE);

              closeMatchWin = TRUE;
            }
            else if(ConvertKey(imsg) == ',')
            {
              // call NormalizeSelection() to make sure that
              // no ' >> ' marks are kept when the user wants to continue
              // with either the next/prev or by simply pressing left/right
              // to finish the selection.
              NormalizeSelection(obj, TRUE);

              closeMatchWin = TRUE;
            }
          }

          // call the SuperMethod now
          result = DoSuperMethodA(cl, obj, msg);

          // if the match window should be closed we don't need to
          // open it beforehand
          if(closeMatchWin == FALSE)
          {
            char *cur_rcpt = (char *)DoMethod(obj, METHOD(CurrentRecipient));
            struct CustomABEntry *abentry;

            if(cur_rcpt != NULL &&
               (abentry = (struct CustomABEntry *)DoMethod(data->Matchwindow, MUIM_AddressmatchPopup_Open, cur_rcpt)) != NULL)
            {
              ULONG pos = xget(obj, MUIA_String_BufferPos);

              // check if the returned entry matches some part of the name (i.e. the last
              // name), but not right from the start of the name
              if(abentry->MatchField == 1 && abentry->RealNameMatchPart > 0)
              {
                struct ABEntry *matchEntry = abentry->MatchEntry;

                if(matchEntry != NULL)
                {
                  // insert " >> name <address>"
                  char address[SIZE_LARGE];

                  strlcpy(address, " >> ", sizeof(address));
                  BuildAddress(&address[4], sizeof(address)-4, matchEntry->Address, data->NoFullName ? NULL : matchEntry->RealName);

                  DoMethod(obj, MUIM_BetterString_Insert, address, pos);

                  xset(obj, MUIA_String_BufferPos, pos,
                            MUIA_BetterString_SelectSize, strlen(address));
                }
              }
              else
              {
                // insert the matching string
                char *new_address = UnquoteString(abentry->MatchString, TRUE);
                ULONG start = DoMethod(obj, METHOD(RecipientStart));

                DoMethod(obj, MUIM_BetterString_Insert, &new_address[pos - start], pos);

                xset(obj, MUIA_String_BufferPos, pos,
                          MUIA_BetterString_SelectSize, strlen(new_address) - (pos - start));

                free(new_address);
              }
            }
          }
          else
          {
            // close the match window if it is open
            if(xget(data->Matchwindow, MUIA_Window_Open) == TRUE)
              set(data->Matchwindow, MUIA_Window_Open, FALSE);
          }
        }
        break;
      }
    }
    else if(imsg->Class == IDCMP_CHANGEWINDOW)
    {
      // only if the matchwindow is open we advice the matchwindow to refresh it`s position.
      if(xget(data->Matchwindow, MUIA_Window_Open))
        DoMethod(data->Matchwindow, MUIM_AddressmatchPopup_ChangeWindow);
    }
    #if defined(__amigaos4__)
    else if(imsg->Class == IDCMP_EXTENDEDMOUSE && (imsg->Code & IMSGCODE_INTUIWHEELDATA))
    {
      // we forward OS4 mousewheel events to the addrmatchlist directly
      if(DoMethod(data->Matchwindow, MUIM_AddressmatchPopup_Event, imsg))
        result = MUI_EventHandlerRC_Eat;
    }
    #endif
  }

  RETURN(result);
  return result;
}
///

/* Public Methods */
/// DECLARE(Resolve)
/* resolve all addresses */
DECLARE(Resolve) // ULONG flags
{
  GETDATA;
  BOOL list_expansion;
  LONG max_list_nesting = 5;
  BOOL res = TRUE;
  BOOL withrealname = TRUE;
  BOOL checkvalids = TRUE;
  BOOL withcache = TRUE;
  BOOL quiet;
  ULONG result;

  ENTER();

  // if this object doesn't have a renderinfo we are quiet
  quiet = muiRenderInfo(obj) == NULL ? TRUE : FALSE;

  // Lets check the flags first
  if(hasNoFullNameFlag(msg->flags) || data->NoFullName)
    withrealname = FALSE;

  if(hasNoValidFlag(msg->flags) || data->NoValid)
    checkvalids = FALSE;

  if(hasNoCacheFlag(msg->flags) || data->NoCache)
    withcache = FALSE;

  set(G->AB->GUI.LV_ADDRESSES, MUIA_NListtree_FindUserDataHook, &FindAddressHook);

  do
  {
    struct MUI_NListtree_TreeNode *tn;
    struct ABEntry *entry;
    BOOL quote = FALSE;
    char *contents;
    char *s;
    char *tmp;

    list_expansion = FALSE;
    s = (STRPTR)xget(obj, MUIA_String_Contents);
    if((contents = tmp = strdup(s)) == NULL)
      break;

    // clear the string gadget without notifying others
    nnset(obj, MUIA_String_Contents, NULL);

    D(DBF_GUI, "resolve this string '%s'", tmp);
    // tokenize string and resolve each recipient
    while((s = rcptok(tmp, &quote)) != NULL)
    {
      char *marks;

      D(DBF_GUI, "token '%s' (quoted %ld)", s, quote);

      // if the resolve string is empty we skip it and go on
      if(s[0] == '\0')
      {
        tmp=NULL;
        continue;
      }

      // skip the " >> " marks if they have been inserted before
      if((marks = strstr(s, " >> ")) != NULL)
      {
        D(DBF_GUI, "skipping marks");
        s = marks + 4;
        D(DBF_GUI, "new token '%s'", s);
      }
      else if(strnicmp(s, "mailto:", 7) == 0)
      {
        D(DBF_GUI, "stripping 'mailto:' header");
        s = &s[7];
        D(DBF_GUI, "new token '%s'", s);
      }

      if(checkvalids == FALSE && (tmp = strchr(s, '@')) != NULL)
      {
        D(DBF_GUI, "valid address found.. will not resolve it '%s'", s);
        DoMethod(obj, METHOD(AddRecipient), s);

        // check if email address lacks domain...
        // and add the one from the currently active user identity
        // or fallback to the default one
        if(tmp[1] == '\0')
        {
          struct UserIdentityNode *uin = data->identity;

          if(uin == NULL)
            uin = GetUserIdentity(&C->userIdentityList, 0, TRUE);

          DoMethod(obj, MUIM_BetterString_Insert, strchr(uin->address, '@')+1, MUIV_BetterString_Insert_EndOfString);
        }
      }
      else if((tn = (struct MUI_NListtree_TreeNode *)DoMethod(G->AB->GUI.LV_ADDRESSES, MUIM_NListtree_FindUserData, MUIV_NListtree_FindUserData_ListNode_Root, s, MUIF_NONE))) /* entry found in address book */
      {
        struct MUI_NListtree_TreeNode *nexttn = (struct MUI_NListtree_TreeNode *)DoMethod(G->AB->GUI.LV_ADDRESSES, MUIM_NListtree_GetEntry, tn, MUIV_NListtree_GetEntry_Position_Next, MUIF_NONE);

        D(DBF_GUI, "found match '%s'", s);

        entry = (struct ABEntry *)tn->tn_User;

        // Now we have to check if there exists another entry in the AB with this string
        if(nexttn == NULL || DoMethod(G->AB->GUI.LV_ADDRESSES, MUIM_NListtree_FindUserData, nexttn, s, MUIV_NListtree_FindUserData_Flag_StartNode) == (ULONG)NULL)
        {
          if(entry->Type == AET_USER) /* it's a normal person */
          {
            D(DBF_GUI, "\tplain user '%s' <%s>", entry->RealName, entry->Address);

            if(withrealname == TRUE && entry->RealName[0] != '\0')
            {
              char address[SIZE_LARGE];

              BuildAddress(address, sizeof(address), entry->Address, entry->RealName);
              DoMethod(obj, METHOD(AddRecipient), address);
            }
            else
              DoMethod(obj, METHOD(AddRecipient), entry->Address);
          }
          else if(entry->Type == AET_LIST) /* it's a list of persons */
          {
            if(data->MultipleRecipients == TRUE)
            {
              char *members;

              if((members = strdup(entry->Members)) != NULL)
              {
                char *lf;

                while((lf = strchr(members, '\n')) != NULL)
                  lf[0] = ',';

                D(DBF_GUI, "found list '%s'", members);
                DoMethod(obj, METHOD(AddRecipient), members);
                free(members);

                if(data->ReplyTo != NULL && entry->Address[0] != '\0')
                  set(data->ReplyTo, MUIA_String_Contents, entry->Address);

                list_expansion = TRUE;
              }
            }
            else
            {
              D(DBF_GUI, "string doesn't allow multiple recipients");
              DoMethod(obj, METHOD(AddRecipient), s);
              res = FALSE;
            }
          }
          else /* it's unknown... */
          {
            D(DBF_GUI, "found matching entry in address book with unknown type %ld", entry->Type);
            DoMethod(obj, METHOD(AddRecipient), s);
            if(quiet == FALSE)
              set(_win(obj), MUIA_Window_ActiveObject, obj);
            res = FALSE;
          }
        }
        else
        {
          D(DBF_GUI, "found more than one matching entry in address book!");
          DoMethod(obj, METHOD(AddRecipient), s);
          if(quiet == FALSE)
            set(_win(obj), MUIA_Window_ActiveObject, obj);
          res = FALSE;
        }
      }
      else if(withcache == TRUE && (entry = (struct ABEntry *)DoMethod(G->App, MUIM_YAMApplication_FindEmailCacheMatch, s)) != NULL)
      {
        D(DBF_GUI, "\temail cache Hit '%s' <%s>", entry->RealName, entry->Address);
        if(withrealname == TRUE && entry->RealName[0] != '\0')
        {
          char address[SIZE_LARGE];

          BuildAddress(address, sizeof(address), entry->Address, entry->RealName);
          DoMethod(obj, METHOD(AddRecipient), address);
        }
        else
          DoMethod(obj, METHOD(AddRecipient), entry->Address);
      }
      else
      {
        D(DBF_GUI, "entry not found '%s'", s);

        if((tmp = strchr(s, '@')) != NULL) /* entry seems to be an email address */
        {
          D(DBF_GUI, "email address '%s'", s);
          DoMethod(obj, METHOD(AddRecipient), s);

          // check if email address lacks domain...
          // and add the one from the currently active user identity
          // or fallback to the default one
          if(tmp[1] == '\0')
          {
            struct UserIdentityNode *uin = data->identity;

            if(uin == NULL)
              uin = GetUserIdentity(&C->userIdentityList, 0, TRUE);

            DoMethod(obj, MUIM_BetterString_Insert, strchr(uin->address, '@')+1, MUIV_BetterString_Insert_EndOfString);
          }
        }
        else
        {
          D(DBF_GUI, "no entry found in addressbook for alias '%s'", s);
          DoMethod(obj, METHOD(AddRecipient), s);
          if(quiet == FALSE)
            set(_win(obj), MUIA_Window_ActiveObject, obj);
          res = FALSE;
        }
      }

      tmp = NULL;
    }

    free(contents);
  }
  while(list_expansion == TRUE && max_list_nesting-- > 0);

  result = (res ? xget(obj, MUIA_String_Contents) : 0);

  RETURN(result);
  return result;
}
///
/// DECLARE(AddRecipient)
/* add a recipient to this string taking care of comma (if in multi-mode). */
DECLARE(AddRecipient) // STRPTR address
{
  GETDATA;
  STRPTR contents;

  ENTER();

  D(DBF_GUI, "add recipient \"%s\"", msg->address);

  // set String_Contents to NULL in case we have
  // a single recipient in the string. Also set the
  // no notify attribute of betterstring to prevent
  // it from sending out two notifications in a row.
  xset(obj, MUIA_BetterString_NoNotify, TRUE,
            data->MultipleRecipients == FALSE ? MUIA_String_Contents : TAG_IGNORE, NULL);

  if((contents = (STRPTR)xget(obj, MUIA_String_Contents)) != NULL && contents[0] != '\0')
    DoMethod(obj, MUIM_BetterString_Insert, ", ", MUIV_BetterString_Insert_EndOfString);

  DoMethod(obj, MUIM_BetterString_Insert, msg->address, MUIV_BetterString_Insert_EndOfString);

  // set bufferpos and reset NoNotify to FALSE
  // so that a notification is triggered
  xset(obj, MUIA_String_BufferPos, -1,
            MUIA_BetterString_NoNotify, FALSE);

  RETURN(0);
  return 0;
}
///
/// DECALRE(RecipientStart)
// return the index where the current recipient start
// (from cursor pos), this is only useful for objects
// with more than one recipient or quotation chars
DECLARE(RecipientStart)
{
  STRPTR buf;
  ULONG pos;
  ULONG i;
  BOOL quote = FALSE;

  ENTER();

  buf = (STRPTR)xget(obj, MUIA_String_Contents);
  pos = xget(obj, MUIA_String_BufferPos);

  for(i=0; i < pos; i++)
  {
    if(buf[i] == '"')
      quote ^= TRUE;
  }

  while(i > 0 && (buf[i-1] != ',' || quote == TRUE))
  {
    i--;

    if(buf[i] == '"')
      quote ^= TRUE;
  }

  while(isspace(buf[i]))
    i++;

  RETURN(i);
  return i;
}
///
/// DECLARE(CurrentRecipient)
/* return current recipient if cursor is at the end of it (i.e at comma or '\0'-byte */
DECLARE(CurrentRecipient)
{
  GETDATA;
  STRPTR buf, end;
  LONG pos;

  ENTER();

  free(data->CurrentRecipient);
  data->CurrentRecipient = NULL;

  buf = (STRPTR)xget(obj, MUIA_String_Contents);
  pos = xget(obj, MUIA_String_BufferPos);

  if((buf[pos] == '\0' || buf[pos] == ',') &&
     (data->CurrentRecipient = strdup(&buf[DoMethod(obj, METHOD(RecipientStart))])) &&
     (end = strchr(data->CurrentRecipient, ',')))
  {
    end[0] = '\0';
  }

  RETURN(data->CurrentRecipient);
  return (ULONG)data->CurrentRecipient;
}
///
/// DECLARE(ReplaceSelected)
DECLARE(ReplaceSelected) // char *address
{
  GETDATA;
  char *new_address = msg->address;
  char *old;
  char *ptr;
  long start;
  long pos;
  long len;

  ENTER();

  D(DBF_GUI, "replace selected: '%s'", new_address);

  if(data->isActive == FALSE && data->selectSize != 0)
  {
    // The user has clicked with the mouse in the match list the string object has gone inactive before
    // which deleted the formerly marked block. This block must be restored as it will be replaced here.
    xset(obj, MUIA_String_BufferPos, data->bufferPos,
              MUIA_BetterString_SelectSize, data->selectSize);

    // forget the remembered selection size again
    data->selectSize = 0;
  }

  // make sure to stop betterstring from sending notifications
  set(obj, MUIA_BetterString_NoNotify, TRUE);

  // we first have to clear the selected area
  DoMethod(obj, MUIM_BetterString_ClearSelected);

  start = DoMethod(obj, METHOD(RecipientStart));
  old = (char *)xget(obj, MUIA_String_Contents);

  // try to find out the length of our current recipient
  if((ptr = strchr(&old[start], ',')) != NULL)
    len = ptr-(&old[start]);
  else
    len = strlen(&old[start]);

  if(Strnicmp(new_address, &old[start], len) != 0)
  {
    xset(obj, MUIA_String_BufferPos,        start,
              MUIA_BetterString_SelectSize, len);

    DoMethod(obj, MUIM_BetterString_ClearSelected);

    start = DoMethod(obj, MUIM_Recipientstring_RecipientStart);
  }
  pos = xget(obj, MUIA_String_BufferPos);

  DoMethod(obj, MUIM_BetterString_Insert, &new_address[pos - start], pos);

  xset(obj, MUIA_String_BufferPos, pos,
            MUIA_BetterString_SelectSize, strlen(new_address) - (pos - start));

  // make sure to turn notifications back on
  set(obj, MUIA_BetterString_NoNotify, FALSE);

  RETURN(0);
  return 0;
}
///
