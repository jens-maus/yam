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

#include "Searchwindow.h"

/* CLASSDATA
struct Data
{
	Object *Searchstring;
	Object *Texteditor;
	Object *ParentWindow;
	ULONG CaseSensitive;
};
*/

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
	struct Data *data;
	Object *string, *case_sensitive, *search, *top, *cancel;

	if (!(obj = DoSuperNew(cl, obj,

		MUIA_Window_Title, GetStr(MSG_SEARCHWINDOW_TITLE),
		WindowContents, VGroup,

			Child, string = BetterStringObject,
				StringFrame,
			End,

			Child, HGroup,
				Child, RectangleObject, End,
				Child, MakeCheckGroup(&case_sensitive, GetStr(MSG_SEARCHWINDOW_TOGGLE_CASESENSITVE)),
				Child, RectangleObject, End,
			End,

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

	DoMethod(string,         MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime, MUIV_Notify_Window, 2, MUIM_Searchwindow_Search, FALSE);
	DoMethod(case_sensitive, MUIM_Notify, MUIA_Selected, MUIV_EveryTime, MUIV_Notify_Self, 3, MUIM_WriteLong, MUIV_TriggerValue, &data->CaseSensitive);
	DoMethod(search,         MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Window, 2, MUIM_Searchwindow_Search, FALSE);
	DoMethod(top,            MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Window, 2, MUIM_Searchwindow_Search, TRUE);
	DoMethod(cancel,         MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Window, 1, MUIM_Searchwindow_Close);
	DoMethod(obj,            MUIM_Notify, MUIA_Window_CloseRequest, TRUE, MUIV_Notify_Self, 1, MUIM_Searchwindow_Close);

	return (ULONG)obj;
}

///

/* Private Functions */

/* Public Methods */
/// DECLARE(Open)
DECLARE(Open) // Object *texteditor
{
	GETDATA;
	STRPTR contents;

	if(data->ParentWindow)
		DoMethod(obj, MUIM_Searchwindow_Close);

	data->Texteditor = msg->texteditor;
	data->ParentWindow = _win(msg->texteditor);

	DoMethod(data->ParentWindow, MUIM_Notify, MUIA_Window_Open, FALSE, obj, 1, MUIM_Searchwindow_Close);

	get(data->Searchstring, MUIA_String_Contents, &contents);
	SetAttrs(data->Searchstring,
		MUIA_String_BufferPos, 0,
		MUIA_BetterString_SelectSize, strlen(contents),
		TAG_DONE);

	SetAttrs(obj,
		MUIA_Window_Activate, TRUE,
		MUIA_Window_ActiveObject, data->Searchstring,
		MUIA_Window_RefWindow, data->ParentWindow,
		MUIA_Window_Open, TRUE,
		TAG_DONE);

	return 0;
}

///
/// DECLARE(Close)
DECLARE(Close)
{
	GETDATA;

	set(obj, MUIA_Window_Open, FALSE);
	DoMethod(data->ParentWindow, MUIM_KillNotifyObj, MUIA_Window_Open, obj);
	data->ParentWindow = NULL;

	return 0;
}

///
/// DECLARE(Search)
DECLARE(Search) // ULONG top
{
	GETDATA;
	STRPTR string;
	Object *parent = data->ParentWindow;

	DoMethod(obj, MUIM_Searchwindow_Close);

	if((get(data->Searchstring, MUIA_String_Contents, &string), string) && string[0] != '\0' && data->Texteditor)
	{
		ULONG flags = 0;
		if(msg->top)             flags |= MUIF_TextEditor_Search_FromTop;
		if(data->CaseSensitive)  flags |= MUIF_TextEditor_Search_CaseSensitive;
		if(!DoMethod(data->Texteditor, MUIM_TextEditor_Search, string, flags))
			MUI_Request(_app(obj), parent, 0L, GetStr(MSG_SEARCHNOTFOUND_TITLE), GetStr(MSG_SEARCHNOTFOUND_BUTTON), GetStr(MSG_SEARCHNOTFOUND_MSG), string);
	}

	return 0;
}

///
/// DECLARE(Next)
DECLARE(Next)
{
	return DoMethod(obj, MUIM_Searchwindow_Search, FALSE);
}

///

