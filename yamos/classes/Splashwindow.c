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
 Description: Splash/Startup window of the application

***************************************************************************/

#include "Splashwindow_cl.h"

/* CLASSDATA
struct Data
{
	Object *statusGauge;
	Object *progressGauge;
	Object *selectGroup;
	Object *userGroup;
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
	Object *statusGauge;
	Object *progressGauge;
	Object *selectGroup;
	Object *userGroup;
	Object *bt_gopage;

	compileInfo = (char *)xget(G->App, MUIA_YAM_CompileInfo);
	strmfp(logopath, G->ProgDir, "Icons/logo");

	if(!(obj = DoSuperNew(cl, obj,

		MUIA_Window_DragBar,		FALSE,
		MUIA_Window_CloseGadget,FALSE,
		MUIA_Window_DepthGadget,FALSE,
		MUIA_Window_SizeGadget,	FALSE,
		MUIA_Window_LeftEdge,		MUIV_Window_LeftEdge_Centered,
		MUIA_Window_TopEdge,		MUIV_Window_TopEdge_Centered,
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
			Child, statusGauge = GaugeObject,
				GaugeFrame,
				MUIA_Gauge_InfoText, " ",
				MUIA_Gauge_Horiz,    TRUE,
			End,
			Child, progressGauge = GaugeObject,
				GaugeFrame,
				MUIA_ShowMe,         FALSE,
				MUIA_Gauge_InfoText, " ",
				MUIA_Gauge_Horiz,    TRUE,
			End,
			Child, selectGroup = VGroup,
				MUIA_ShowMe, FALSE,
				Child, RectangleObject,
					 MUIA_Rectangle_HBar, TRUE,
					 MUIA_FixHeight, 8,
				End,
				Child, userGroup = VGroup,
					Child, TextObject,
						MUIA_Text_Contents, GetStr(MSG_UserLogin),
						MUIA_Text_PreParse, MUIX_C,
					End,
				End,
				Child, HVSpace,
			End,
		End,

		TAG_MORE, (ULONG)inittags(msg))))
	{
		return 0;
	}

	if(!(data = (struct Data *)INST_DATA(cl,obj)))
		return 0;

	data->statusGauge = statusGauge;
	data->progressGauge = progressGauge;
	data->selectGroup = selectGroup;
	data->userGroup = userGroup;

	DoMethod(G->App, OM_ADDMEMBER, obj);

	DoMethod(bt_gopage, MUIM_Notify, MUIA_Pressed, FALSE, obj, 2, MUIM_CallHook, &OpenSupportPageHook);

	set(obj, MUIA_Window_Activate, TRUE);

	return (ULONG)obj;
}

///

/* Private Functions */

/* Public Methods */
/// DECLARE(StatusChange)
DECLARE(StatusChange) // char *txt, LONG percent
{
	GETDATA;

	if(msg->txt && msg->percent >= 0)
	{
		SetAttrs(data->statusGauge, MUIA_Gauge_InfoText, msg->txt,
																MUIA_Gauge_Current,  msg->percent,
						 TAG_DONE);
	}
	else if(msg->txt)
		set(data->statusGauge, MUIA_Gauge_InfoText, msg->txt);
	else
		set(data->statusGauge, MUIA_Gauge_Current, msg->percent);

	nnset(data->progressGauge, MUIA_ShowMe, FALSE);

	DoMethod(G->App, MUIM_Application_InputBuffered);

	return 0;
}

///
/// DECLARE(ProgressChange)
DECLARE(ProgressChange) // char *txt, LONG percent, LONG max
{
	GETDATA;

	if(msg->txt)
		set(data->progressGauge, MUIA_Gauge_InfoText, msg->txt);

	if(msg->percent >= 0)
		set(data->progressGauge, MUIA_Gauge_Current, msg->percent);

	if(msg->max >= 0)
		set(data->progressGauge, MUIA_Gauge_Max, msg->max);

	set(data->progressGauge, MUIA_ShowMe, TRUE);

	DoMethod(G->App, MUIM_Application_InputBuffered);

	return 0;
}

///
/// DECLARE(SelectUser)
DECLARE(SelectUser)
{
	GETDATA;
	int user = -1;

	if(DoMethod(data->userGroup, MUIM_Group_InitChange))
	{
		Object *button0 = NULL;
		Object *group = ColGroup(2), End;
		int i;
		BOOL wasIconified;
		BOOL wasOpen;

		for(i = 0; i < G->Users.Num; i++)
		{
			Object *button = MakeButton(G->Users.User[i].Name);
			
			if(!i)
				button0 = button;
			
			DoMethod(group, OM_ADDMEMBER, button);
			DoMethod(button, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 2, MUIM_Application_ReturnID, ID_LOGIN+i);
		}
		
		if(i%2 == 1)
			DoMethod(group, OM_ADDMEMBER, HSpace(0));
		
		DoMethod(data->userGroup, OM_ADDMEMBER, group);
		DoMethod(data->userGroup, MUIM_Group_ExitChange);
		set(data->statusGauge, MUIA_Gauge_InfoText, GetStr(MSG_US_WaitLogin));
		
		set(data->selectGroup, MUIA_ShowMe, TRUE);
		SetAttrs(obj, MUIA_Window_ActiveObject,  button0,
									MUIA_Window_DefaultObject, button0,
						 TAG_DONE);

		// make sure the window is open and not iconified
		wasOpen = xget(obj, MUIA_Window_Open);
		wasIconified = xget(G->App, MUIA_Application_Iconified);

		if(!wasOpen)
			set(obj, MUIA_Window_Open, TRUE);

		if(wasIconified)
			set(G->App, MUIA_Application_Iconified, FALSE);
		
		// make sure the window is at the front
		DoMethod(obj, MUIM_Window_ToFront);

		// lets collect the waiting returnIDs now
		COLLECT_RETURNIDS;

		do
		{
      ULONG signals;
			LONG ret = DoMethod(G->App, MUIM_Application_NewInput, &signals)-ID_LOGIN;

      // bail out if a button was hit
			if(ret >= 0 && ret < G->Users.Num)
			{
				user = ret;
				break;
			}

			if(signals)
				Wait(signals);
		}
		while(1);

		// now lets reissue the collected returnIDs again
		REISSUE_RETURNIDS;

		// lets iconify/close the window if it had this state previously
		if(!wasOpen)
			set(obj, MUIA_Window_Open, FALSE);

		if(wasIconified)
			set(G->App, MUIA_Application_Iconified, TRUE);

		set(data->selectGroup, MUIA_ShowMe, FALSE);

		DoMethod(data->userGroup, MUIM_Group_InitChange);
		DoMethod(data->userGroup, OM_REMMEMBER, group);
		DoMethod(data->userGroup, MUIM_Group_ExitChange);
	}

	return user;
}

///
