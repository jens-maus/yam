/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2004 by YAM Open Source Team

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
 Description: About window of the application

***************************************************************************/

#include "Aboutwindow_cl.h"

/* CLASSDATA
struct Data
{
	char *aboutText;
};
*/

/* Hooks */
/// OpenSupportPageHook
//  User clicked homepage URL in About window
HOOKPROTONHNONP(OpenSupportPageFunc, void)
{
	GotoURL("http://www.yam.ch/");
}
MakeStaticHook(OpenSupportPageHook, OpenSupportPageFunc);
///

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
	struct Data *data;
	char logopath[SIZE_PATHFILE];
	char *compileInfo;
	char *aboutText;
	Object *bt_okay;
	Object *bt_gopage;

	compileInfo = (char *)xget(G->App, MUIA_YAM_CompileInfo);
	strmfp(logopath, G->ProgDir, "Icons/logo");

	// now we create the about text
	aboutText = AllocStrBuf(SIZE_LARGE);
	aboutText = StrBufCat(aboutText, GetStr(MSG_Copyright2));
	aboutText = StrBufCat(aboutText, GetStr(MSG_UsedSoftware));
	aboutText = StrBufCat(aboutText, "\0338Magic User Interface\0332 (Stefan Stuntz)\n"
																	 "\0338TextEditor.mcc, BetterString.mcc\0332 (Allan Odgaard)\n"
																	 "\0338Toolbar.mcc\0332 (Benny Kjær Nielsen)\n"
																	 "\0338NList.mcc, NListview.mcc\0332 (Gilles Masson)\n"
																	 "\0338NListtree.mcc\0332 (Carsten Scholling)\n"
																	 "\0338XPK\0332 (Urban D. Müller, Dirk Stöcker)\n"
																	 "\0338amissl.library\0332 (Andrija Antonijevic)\n\n");
	aboutText = StrBufCat(aboutText, GetStr(MSG_WebSite));

	if(!(obj = DoSuperNew(cl, obj,

		MUIA_Window_Title, GetStr(MSG_MA_About),
		MUIA_Window_ID, MAKE_ID('A','B','T','0'),
		MUIA_HelpNode, "ABOUT",
		WindowContents, VGroup,
			MUIA_Background, MUII_GroupBack,
			Child, HGroup,
				MUIA_Group_Spacing, 0,
				Child, HSpace(0),
				Child, NewObject(CL_BodyChunk->mcc_Class,NULL,
					MUIA_Bodychunk_File, logopath,
				End,
				Child, HSpace(0),
			End,
			Child, HCenter((VGroup,
				Child, CLabel(GetStr(MSG_Copyright1)),
				Child, ColGroup(2),
					Child, bt_gopage = TextObject,
						MUIA_Text_Contents, "\033c\033u\0335http://www.yam.ch/",
						MUIA_InputMode, MUIV_InputMode_RelVerify,
					End,
				End,
				Child, RectangleObject,
					 MUIA_Rectangle_HBar, TRUE,
					 MUIA_FixHeight, 8,
				End,
				Child, ColGroup(2),
					 MUIA_Group_HorizSpacing, 8,
					 MUIA_Group_VertSpacing, 2,
					 Child, Label(GetStr(MSG_Version)),
					 Child, LLabel(yamversionver),
					 Child, Label(GetStr(MSG_CompilationDate)),
					 Child, LLabel(compileInfo),
				End,
			End)),
			Child, ListviewObject,
				MUIA_Listview_Input, FALSE,
				MUIA_Listview_List, FloattextObject,
					ReadListFrame,
					MUIA_Floattext_Text, aboutText,
				End,
			End,
			Child, HGroup,
        Child, RectangleObject, End,
        Child, bt_okay = SimpleButton(GetStr(MSG_ABOUT_OKAY_GAD)),
        Child, RectangleObject, End,
			End,
		End,

		TAG_MORE, (ULONG)inittags(msg))))
	{
		return 0;
	}

	if(!(data = (struct Data *)INST_DATA(cl,obj)))
		return 0;

	DoMethod(G->App, OM_ADDMEMBER, obj);

	// copy the pointers to our instance data structure
	data->aboutText = aboutText;

	DoMethod(obj, 			MUIM_Notify, MUIA_Window_CloseRequest, TRUE, obj, 3, MUIM_Set, MUIA_Window_Open, FALSE);
	DoMethod(bt_okay, 	MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Window, 3, MUIM_Set, MUIA_Window_Open, FALSE);
	DoMethod(bt_gopage, MUIM_Notify, MUIA_Pressed, FALSE, obj, 2, MUIM_CallHook, &OpenSupportPageHook);

	SetAttrs(obj,
		MUIA_Window_Activate, 		 TRUE,
		MUIA_Window_DefaultObject, bt_okay,
	TAG_DONE);

	return (ULONG)obj;
}

///
/// OVERLOAD(OM_DISPOSE)
OVERLOAD(OM_DISPOSE)
{
	GETDATA;

	if(data->aboutText)
		FreeStrBuf(data->aboutText);

	return DoSuperMethodA(cl, obj, msg);
}

///

/* Private Functions */

/* Public Methods */
