/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2001 by YAM Open Source Team

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 YAM Official Support Site : http://www.yam.ch
 YAM OpenSource project		 : http://sourceforge.net/projects/yamos/

 $Id$

 Superclass:  MUIC_BetterString
 Description: Auto-completes email addresses etc.

***************************************************************************/

#include "Recipientstring.h"

/* CLASSDATA
struct Data
{
	struct MUI_EventHandlerNode ehnode;
	Object *Matchwindow;                //, *Matchlist;
	Object *From, *ReplyTo;             // used when resolving a list address
	STRPTR CurrentRecipient;
	BOOL MultipleRecipients;
	BOOL ResolveOnCR;
};
*/

/* EXPORT
#define MUIF_Recipientstring_Resolve_NoFullName  (1 << 0) // do not resolve with fullname "Mister X <misterx@mister.com>"
#define MUIF_Recipientstring_Resolve_NoValid     (1 << 1) // do not resolve already valid string like "misterx@mister.com"
*/


/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
	if(obj = (Object *)DoSuperMethodA(cl, obj, msg))
	{
		GETDATA;

		struct TagItem *tags = inittags(msg), *tag;
		while((tag = NextTagItem(&tags)))
		{
			switch(tag->ti_Tag)
			{
				ATTR(ResolveOnCR)        : data->ResolveOnCR = tag->ti_Data        ; break;
				ATTR(MultipleRecipients) : data->MultipleRecipients = tag->ti_Data ; break;
				ATTR(FromString)         : data->From = (Object *)tag->ti_Data     ; break;
				ATTR(ReplyToString)      : data->ReplyTo = (Object *)tag->ti_Data  ; break;
			}
		}

		SetAttrs(obj,
			MUIA_String_Popup, obj,
			MUIA_String_Reject, data->MultipleRecipients ? NULL : ",",
			TAG_DONE);
	}
	return (ULONG)obj;
}
///

/// OVERLOAD(OM_DISPOSE)
OVERLOAD(OM_DISPOSE)
{
	GETDATA;

	if(data->Matchwindow)
	{
		DB(kprintf("Dispose addrlistpopup: %lx\n", data->Matchwindow);)
		DoMethod(_app(obj), OM_REMMEMBER, data->Matchwindow);
		MUI_DisposeObject(data->Matchwindow);
	}

	free(data->CurrentRecipient);
	return DoSuperMethodA(cl, obj, msg);
}
///

/// OVERLOAD(OM_GET)
/* this is just so that we can notify the popup tag */
OVERLOAD(OM_GET)
{
	ULONG *store = ((struct opGet *)msg)->opg_Storage;
	switch(((struct opGet *)msg)->opg_AttrID)
	{
		ATTR(Popup) : *store = FALSE ; return TRUE;
	}
	return DoSuperMethodA(cl, obj, msg);
}
///

/// OVERLOAD(OM_SET)
OVERLOAD(OM_SET)
{
	GETDATA;

	struct TagItem *tags = inittags(msg), *tag;
	while((tag = NextTagItem(&tags)))
	{
		switch(tag->ti_Tag)
		{
			ATTR(ResolveOnCR)        : data->ResolveOnCR = tag->ti_Data        ; break;
			ATTR(MultipleRecipients) : data->MultipleRecipients = tag->ti_Data ; break;
			ATTR(FromString)         : data->From = (Object *)tag->ti_Data     ; break;
			ATTR(ReplyToString)      : data->ReplyTo = (Object *)tag->ti_Data  ; break;
		}
	}

	return DoSuperMethodA(cl, obj, msg);
}
///

/// OVERLOAD(MUIM_Setup)
OVERLOAD(MUIM_Setup)
{
	GETDATA;

	if(!data->Matchwindow && (data->Matchwindow = AddrmatchlistObject, MUIA_Addrmatchlist_String, obj, End))
	{
		DB(kprintf("Create addrlistpopup: %lx\n", data->Matchwindow);)
		DoMethod(_app(obj), OM_ADDMEMBER, data->Matchwindow);
	}

	if(data->Matchwindow && DoSuperMethodA(cl, obj, msg))
	{
		data->ehnode.ehn_Priority = 1;
		data->ehnode.ehn_Flags	  = 0;
		data->ehnode.ehn_Object	  = obj;
		data->ehnode.ehn_Class	  = cl;
		data->ehnode.ehn_Events	  = IDCMP_RAWKEY | IDCMP_CHANGEWINDOW;
		return TRUE;
	}
	return FALSE;
}
///

/// OVERLOAD(MUIM_Show)
OVERLOAD(MUIM_Show)
{
	GETDATA;
	DoMethod(data->Matchwindow, MUIM_Addrmatchlist_ChangeWindow);
	return DoSuperMethodA(cl, obj, msg);
}
///

/// OVERLOAD(MUIM_GoActive)
OVERLOAD(MUIM_GoActive)
{
	GETDATA;
	DoMethod(_win(obj), MUIM_Window_AddEventHandler, &data->ehnode);
	return DoSuperMethodA(cl, obj, msg);
}
///

/// OVERLOAD(MUIM_GoInactive)
OVERLOAD(MUIM_GoInactive)
{
	GETDATA;
	DoMethod(_win(obj), MUIM_Window_RemEventHandler, &data->ehnode);
	set(data->Matchwindow, MUIA_Window_Open, FALSE);
	return DoSuperMethodA(cl, obj, msg);
}
///

/// OVERLOAD(MUIM_DragQuery)
OVERLOAD(MUIM_DragQuery)
{
	struct MUIP_DragQuery *d = (struct MUIP_DragQuery *)msg;
	ULONG result = MUIV_DragQuery_Refuse;
	if(d->obj == G->MA->GUI.NL_MAILS)
	{
		result = MUIV_DragQuery_Accept;
	}
	else if(d->obj == G->AB->GUI.LV_ADDRESSES)
	{
		struct MUI_NListtree_TreeNode *active;
		if(active = (struct MUI_NListtree_TreeNode *)xget(d->obj, MUIA_NListtree_Active))
		{
			if(!(active->tn_Flags & TNF_LIST))
				result = MUIV_DragQuery_Accept;
		}
	}
	return result;
}
///

/// OVERLOAD(MUIM_DragDrop)
OVERLOAD(MUIM_DragDrop)
{
	struct MUIP_DragQuery *d = (struct MUIP_DragQuery *)msg;
	if(d->obj == G->MA->GUI.NL_MAILS)
	{
		struct Mail *mail;
		DoMethod(d->obj, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &mail);
		if(OUTGOING(mail->Folder->Type))
				AB_InsertAddress(obj, "", mail->To.RealName,   mail->To.Address);
		else	AB_InsertAddress(obj, "", mail->From.RealName, mail->From.Address);
	}
	else if(d->obj == G->AB->GUI.LV_ADDRESSES)
	{
		struct MUI_NListtree_TreeNode *active = (struct MUI_NListtree_TreeNode *)xget(d->obj, MUIA_NListtree_Active);
		struct ABEntry *addr = (struct ABEntry *)(active->tn_User);
		AB_InsertAddress(obj, addr->Alias, addr->RealName, "");
	}
	return 0;
}
///

/// OVERLOAD(MUIM_Popstring_Open)
/* this method is invoked when the MUI popup key is pressed, we let it trigger a notify, so that the address book will open -- in the future this should be removed and we should just use a Popupstring object */
OVERLOAD(MUIM_Popstring_Open)
{
	set(obj, MUIA_Recipientstring_Popup, TRUE);
	return 0;
}
///

/// OVERLOAD(MUIM_HandleEvent)
OVERLOAD(MUIM_HandleEvent)
{
	GETDATA;
	ULONG result = 0;
	struct IntuiMessage *imsg;
	if(!(imsg = ((struct MUIP_HandleEvent *)msg)->imsg))
		return 0;

	if(imsg->Class == IDCMP_RAWKEY)
	{
		STRPTR new_address = NULL;
		switch(imsg->Code)
		{
			case IECODE_RETURN:
			{
				if(data->ResolveOnCR)
				{
					DoMethod(obj, MUIM_Recipientstring_Resolve, imsg->Qualifier & (IEQUALIFIER_RSHIFT | IEQUALIFIER_LSHIFT) ? MUIF_Recipientstring_Resolve_NoFullName : MUIF_NONE);

					// If the MUIA_String_AdvanceOnCR is TRUE we have to set the next object active in the window
					if(xget(obj, MUIA_String_AdvanceOnCR))
						set(_win(obj), MUIA_Window_ActiveObject, MUIV_Window_ActiveObject_Next);

					result = MUI_EventHandlerRC_Eat;
				}
			}
			break;

			/* keys are sent to the popup-list */
			case IECODE_UP:
			case IECODE_DOWN:
			{
				if(new_address = (STRPTR)DoMethod(data->Matchwindow, MUIM_Addrmatchlist_Event, imsg))
				{
          LONG start;
          STRPTR old;

          // we first have to clear the selected area
          DoMethod(obj, MUIM_BetterString_ClearSelected);

          start = DoMethod(obj, MUIM_Recipientstring_RecipientStart);
          old = (STRPTR)xget(obj, MUIA_String_Contents);

          if(Strnicmp(new_address, &old[start], strlen(&old[start])) != 0)
          {
            SetAttrs(obj, MUIA_String_BufferPos, start,
                          MUIA_BetterString_SelectSize, strlen(&old[start]),
                          TAG_DONE);

            DoMethod(obj, MUIM_BetterString_ClearSelected);
          }

					result = MUI_EventHandlerRC_Eat;
				}
			}
			break;

			case IECODE_DEL:
			case IECODE_ESCAPE: /* FIXME: Escape should clear the marked text. Currently the marked text goes when leaving the gadget or e.g. pressing ','. Seems to be a refresh problem */
				set(data->Matchwindow, MUIA_Window_Open, FALSE);
			break;

			case IECODE_BACKSPACE:
				DoMethod(obj, MUIM_BetterString_ClearSelected);
			/* continue */

			default:
			{
				STRPTR old, new;

				if(xget(obj, MUIA_BetterString_SelectSize) != 0 && ConvertKey(imsg) == ',')
					set(obj, MUIA_String_BufferPos, MUIV_BetterString_BufferPos_End);

				old = strdup((STRPTR)xget(obj, MUIA_String_Contents));

				DoSuperMethodA(cl, obj, msg);
				new = (STRPTR)xget(obj, MUIA_String_Contents);

				if(strcmp(old, new)) /* if contents changed, check if something matches */
					new_address = (STRPTR)DoMethod(data->Matchwindow, MUIM_Addrmatchlist_Open, DoMethod(obj, MUIM_Recipientstring_CurrentRecipient));

				free(old);

				result = MUI_EventHandlerRC_Eat;
			}
			break;
		}

		if(new_address) /* this is the complete address of what the user is typing, so let's insert it (marked) */
		{
			LONG start = DoMethod(obj, MUIM_Recipientstring_RecipientStart);
			LONG pos = xget(obj, MUIA_String_BufferPos);

			DoMethod(obj, MUIM_BetterString_Insert, &new_address[pos - start], pos);

			SetAttrs(obj, MUIA_String_BufferPos, pos,
                    MUIA_BetterString_SelectSize, strlen(new_address) - (pos - start),
                    TAG_DONE);
		}
	}
	else if(imsg->Class == IDCMP_CHANGEWINDOW)
	{
		DoMethod(data->Matchwindow, MUIM_Addrmatchlist_ChangeWindow);
	}
	return result;
}
///

HOOKPROTONH(FindAddressFunc, LONG, Object *obj, struct MUIP_NListtree_FindUserDataMessage *msg)
{
	struct ABEntry *entry = (struct ABEntry *)msg->UserData;
	return ((entry->Type == AET_USER) || (entry->Type == AET_LIST)) && ((!Stricmp(msg->User, entry->Alias) || !Stricmp(msg->User, entry->RealName) || !Stricmp(msg->User, entry->Address))) ? 0 : ~0;
}
MakeStaticHook(FindAddressHook, FindAddressFunc);

/// DECLARE(Resolve)
/* resolve all addresses */
DECLARE(Resolve) // ULONG flags
{
	GETDATA;
	BOOL list_expansion;
	LONG max_list_nesting = 5;
	STRPTR s, contents, tmp;
  BOOL res = TRUE;
  BOOL withrealname = TRUE, checkvalids = TRUE;

   // Lets check the flags first
   if(msg->flags & MUIF_Recipientstring_Resolve_NoFullName) withrealname= FALSE;
   if(msg->flags & MUIF_Recipientstring_Resolve_NoValid)    checkvalids = FALSE;

	set(G->AB->GUI.LV_ADDRESSES, MUIA_NListtree_FindUserDataHook, &FindAddressHook);

	do {

		struct MUI_NListtree_TreeNode *tn;
		list_expansion = FALSE;
		get(obj, MUIA_String_Contents, &s);
		if(!(contents = tmp = strdup(s)))
			break;
		set(obj, MUIA_String_Contents, NULL);

		DB(kprintf("Resolve this string: %s\n", tmp);)
		while(s = Trim(strtok(tmp, ","))) /* tokenize string and resolve each recipient */
		{
      DB(kprintf("token: '%s'\n", s);)

      // if the resolve string is empty we skip it and go on
      if(!s[0] || strlen(s) == 0)
      {
        tmp=NULL;
        continue;
      }

      if(checkvalids == FALSE && (tmp = strchr(s, '@')))
      {
		    DB(kprintf("Valid address found.. will not resolve it: %s\n", s);)
			  DoMethod(obj, MUIM_Recipientstring_AddRecipient, s);

            /* email address lacks domain... */
   			if(tmp[1] == '\0')
	   		   DoMethod(obj, MUIM_BetterString_Insert, strchr(C->EmailAddress, '@')+1, MUIV_BetterString_Insert_EndOfString);
      }
      else if(tn = (struct MUI_NListtree_TreeNode *)DoMethod(G->AB->GUI.LV_ADDRESSES, MUIM_NListtree_FindUserData, MUIV_NListtree_FindUserData_ListNode_Root, s, MUIF_NONE)) /* entry found in address book */
			{
        struct MUI_NListtree_TreeNode *nexttn = (struct MUI_NListtree_TreeNode *)DoMethod(G->AB->GUI.LV_ADDRESSES, MUIM_NListtree_GetEntry, tn, MUIV_NListtree_GetEntry_Position_Next, MUIF_NONE);
				struct ABEntry *entry = (struct ABEntry *)tn->tn_User;
				DB(kprintf("Found match: %s\n", s);)

        // Now we have to check if there exists another entry in the AB with this string
        if(nexttn == NULL || DoMethod(G->AB->GUI.LV_ADDRESSES, MUIM_NListtree_FindUserData, nexttn, s, MUIV_NListtree_FindUserData_Flag_StartNode) == NULL)
  			{
        	if(entry->Type == AET_USER) /* it's a normal person */
				  {
					  DB(kprintf("\tPlain user: %s (%s, %s)\n", AB_PrettyPrintAddress(entry), entry->RealName, entry->Address);)
  					DoMethod(obj, MUIM_Recipientstring_AddRecipient, withrealname && entry->RealName[0] ? AB_PrettyPrintAddress(entry) : (STRPTR)entry->Address);
	  			}
		  		else if(entry->Type == AET_LIST) /* it's a list of persons */
			  	{
				  	if(data->MultipleRecipients)
					  {
  						STRPTR members, lf;
	  					if(members = strdup(entry->Members))
		  				{
			  				while(lf = strchr(members, '\n'))
				  			   lf[0] = ',';

					  		DB(kprintf("Found list: »%s«\n", entry->Members);)
						  	DoMethod(obj, MUIM_Recipientstring_AddRecipient, members);
							  free(members);

  							if(entry->RealName[0])	set(data->From, MUIA_String_Contents, AB_PrettyPrintAddress2(entry->RealName, C->EmailAddress));
	  						if(entry->Address[0])	set(data->ReplyTo, MUIA_String_Contents, entry->Address);

		  					list_expansion = TRUE;
			  			}
				  	}
					  else
  					{
	  					D(DBF_ERROR, ("String doesn't allow multiple recipients\n"))
		  				DoMethod(obj, MUIM_Recipientstring_AddRecipient, s);
              res = FALSE;
				  	}
				  }
  				else /* it's unknown... */
	  			{
		  			D(DBF_ERROR, ("Found matching entry in address book with unknown type: %ld", entry->Type))
			  		DoMethod(obj, MUIM_Recipientstring_AddRecipient, s);
            set(_win(obj), MUIA_Window_ActiveObject, obj);
            res = FALSE;
  				}
        }
        else
        {
		  	  D(DBF_ERROR, ("Found more than one matching entry in address book!\n"))
			    DoMethod(obj, MUIM_Recipientstring_AddRecipient, s);
          set(_win(obj), MUIA_Window_ActiveObject, obj);
          res = FALSE;
        }
			}
			else
			{
				DB(kprintf("Entry not found: %s\n", s);)

   		   if(tmp = strchr(s, '@')) /* entry seems to be an email address */
	   	   {
		   	   DB(kprintf("Email address: %s\n", s);)
			     DoMethod(obj, MUIM_Recipientstring_AddRecipient, s);

               /* email address lacks domain... */
   			   if(tmp[1] == '\0')
	   			   DoMethod(obj, MUIM_BetterString_Insert, strchr(C->EmailAddress, '@')+1, MUIV_BetterString_Insert_EndOfString);
         }
         else
				 {
				    D(DBF_ERROR, ("No entry found in addressbook for alias: %s", s))
            DoMethod(obj, MUIM_Recipientstring_AddRecipient, s);
            set(_win(obj), MUIA_Window_ActiveObject, obj);
            res = FALSE;
         }
			}

			tmp = NULL;
		}
		free(contents);

	} while(list_expansion && max_list_nesting-- > 0);

	return (ULONG)(res ? xget(obj, MUIA_String_Contents) : NULL);
}
///

/// DECLARE(AddRecipient)
/* add a recipient to this string taking care of comma (if in multi-mode). */
DECLARE(AddRecipient) // STRPTR address
{
	GETDATA;
	STRPTR contents;

	if(!data->MultipleRecipients)
		set(obj, MUIA_String_Contents, NULL);

	if(get(obj, MUIA_String_Contents, &contents), contents[0] != '\0')
		DoMethod(obj, MUIM_BetterString_Insert, ", ", MUIV_BetterString_Insert_EndOfString);

	DoMethod(obj, MUIM_BetterString_Insert, msg->address, MUIV_BetterString_Insert_EndOfString);
	set(obj, MUIA_String_BufferPos, -1);

	return 0;
}
///

/// DECALRE(RecipientStart)
/* return the index where current recipient start (from cursor pos), this is only useful for objects with more than one recipient */
DECLARE(RecipientStart)
{
	STRPTR buf;
	ULONG pos, start;
	get(obj, MUIA_String_Contents, &buf);
	get(obj, MUIA_String_BufferPos, &pos);

	start = pos;
	while(start > 0 && buf[start-1] != ',')
		start--;
	while(ISpace(buf[start]))
		start++;

	return start;
}
///

/// DECLARE(CurrentRecipient)
/* return current recipient if cursor is at the end of it (i.e at comma or '\0'-byte */
DECLARE(CurrentRecipient)
{
	GETDATA;
	STRPTR buf, end;
	LONG pos;

	free(data->CurrentRecipient);
	data->CurrentRecipient = NULL;

	get(obj, MUIA_String_Contents, &buf);
	get(obj, MUIA_String_BufferPos, &pos);
	if((buf[pos] == '\0' || buf[pos] == ',') && (data->CurrentRecipient = strdup(&buf[DoMethod(obj, MUIM_Recipientstring_RecipientStart)])) && (end = strchr(data->CurrentRecipient, ',')))
		end[0] = '\0';

	return (ULONG)data->CurrentRecipient;
}
///
