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
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 YAM Official Support Site :  http://www.yam.ch
 YAM OpenSource project    :  http://sourceforge.net/projects/yamos/

 $Id$

 Superclass:  MUIC_Window
 Description: Window where user inputs search string and options.

***************************************************************************/

#include <clib/alib_protos.h>
#include <libraries/iffparse.h>
#include <libraries/mui.h>
#include <mui/BetterString_mcc.h>
#include <mui/TextEditor_mcc.h>
#include <proto/intuition.h>
#include <proto/muimaster.h>
#include <proto/utility.h>

#include "YAM.h"
#include "YAM_locale.h"
#include "YAM_utilities.h"

/* ---------------------------------- */
#define DECLARE(method) ULONG m_Searchwindow_## method (struct IClass *cl, Object *obj, struct MUIP_Searchwindow_## method *msg)
#define OVERLOAD(method) ULONG m_Searchwindow_## method (struct IClass *cl, Object *obj, Msg msg)
#define ATTR(attr) case MUIA_Searchwindow_## attr
/* ---------------------------------- */

#define inittags(msg) (((struct opSet *)msg)->ops_AttrList)
#define GETDATA struct Data *data = (struct Data *)INST_DATA(cl,obj)

struct Data
{
	Object *Searchstring;
	Object *Texteditor;
	ULONG CaseSensitive;
};

ULONG SearchwindowGetSize (VOID) { return sizeof(struct Data); }

#define MUIM_Searchwindow_Open       0x80000000
#define MUIM_Searchwindow_Search     0x80000001
#define MUIM_Searchwindow_Next       0x80000002
#define MUIA_Searchwindow_Texteditor 0x80000003

OVERLOAD(OM_NEW)
{
	struct TagItem *tags, *tag;
	struct Data *data;
	Object *string, *case_sensitive, *search, *top, *cancel;

	if (!(obj = (Object *)DoSuperNew(cl, obj,

		MUIA_Window_Title, GetStr(MSG_SEARCHWINDOW_TITLE),
		MUIA_Window_ID, MAKE_ID('S','E','A','R'),
		WindowContents, VGroup,

			Child, string = BetterStringObject,
				StringFrame,
			End,

			Child, MakeCheckGroup(&case_sensitive, GetStr(MSG_SEARCHWINDOW_TOGGLE_CASESENSITVE)),

			Child, HGroup,
				Child, search = SimpleButton(GetStr(MSG_SEARCHWINDOW_BT_SEARCH)),
				Child, top    = SimpleButton(GetStr(MSG_SEARCHWINDOW_BT_FROMTOP)),
				Child, cancel = SimpleButton(GetStr(MSG_SEARCHWINDOW_BT_CANCEL)),
			End,

		End,

		TAG_MORE, (ULONG)inittags(msg))))
		return 0;

	data = (struct Data *)INST_DATA(cl,obj);

	data->Searchstring = string;

	tags = inittags(msg);
	while((tag = NextTagItem(&tags)))
	{
		switch(tag->ti_Tag)
		{
			ATTR(Texteditor) : data->Texteditor = (Object *)tag->ti_Data  ; break;
		}
	}

	DoMethod(string,         MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime, MUIV_Notify_Window, 2, MUIM_Searchwindow_Search, FALSE);
	DoMethod(case_sensitive, MUIM_Notify, MUIA_Selected, MUIV_EveryTime, MUIV_Notify_Self, 3, MUIM_WriteLong, MUIV_TriggerValue, &data->CaseSensitive);
	DoMethod(search,         MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Window, 2, MUIM_Searchwindow_Search, FALSE);
	DoMethod(top,            MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Window, 2, MUIM_Searchwindow_Search, TRUE);
	DoMethod(cancel,         MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Window, 3, MUIM_Set, MUIA_Window_Open, FALSE);
	DoMethod(obj,            MUIM_Notify, MUIA_Window_CloseRequest, TRUE, MUIV_Notify_Self, 3, MUIM_Set, MUIA_Window_Open, FALSE);

	return (ULONG)obj;
}

DECLARE(Open)
{
	GETDATA;

	SetAttrs(data->Searchstring,
		MUIA_String_BufferPos, 0,
		MUIA_BetterString_SelectSize, 0xffff,
		TAG_DONE);

	SetAttrs(obj,
		MUIA_Window_ActiveObject, data->Searchstring,
		MUIA_Window_Open, TRUE,
		TAG_DONE);

	return 0;
}

DECLARE(Search) // ULONG top
{
	GETDATA;
	STRPTR string;

	set(obj, MUIA_Window_Open, FALSE);

	if((get(data->Searchstring, MUIA_String_Contents, &string), string) && string[0] != '\0' && data->Texteditor)
	{
		ULONG flags = 0;
		if(msg->top)             flags |= MUIF_TextEditor_Search_FromTop;
		if(data->CaseSensitive)  flags |= MUIF_TextEditor_Search_CaseSensitive;
		DoMethod(data->Texteditor, MUIM_TextEditor_Search, string, flags);
	}

	return 0;
}

DECLARE(Next)
{
	DoMethod(obj, MUIM_Searchwindow_Search, FALSE);
	return 0;
}
