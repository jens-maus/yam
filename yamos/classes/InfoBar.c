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

 Superclass:  MUIC_Group
 Description: Displays additional information in the Main window

***************************************************************************/

#include "InfoBar.h"

/* CLASSDATA
struct Data
{
	Object *TX_FOLDER;
	Object *TX_FINFO;
	Object *TX_INFO;
	Object *BC_INFO;
	Object *GA_GROUP;
	Object *GA_INFO;
	Object *GA_LABEL;
	struct Folder *actualFolder;
};
*/

/* Private Functions */
/// GetFolderInfo()
// this function creates a folder string and returns it
char *GetFolderInfo(struct Folder *folder)
{
	char *src, dst[10];
	static char bartxt[SIZE_DEFAULT/2];

	// clear the bar text first
	strcpy(bartxt, "");

	// Lets create the label of the AppIcon now
	for (src = C->InfoBarText; *src; src++)
	{
		if (*src == '%')
		{
			switch (*++src)
			{
				case '%': strcpy(dst, "%");                     break;
				case 'n': sprintf(dst, "%ld", folder->New);     break;
				case 'u': sprintf(dst, "%ld", folder->Unread);  break;
				case 't': sprintf(dst, "%ld", folder->Total);   break;
				case 's': sprintf(dst, "%ld", folder->Sent);    break;
				case 'd': sprintf(dst, "%ld", folder->Deleted); break;
			}
		}
		else
		{
			sprintf(dst, "%c", *src);
		}

		strcat(bartxt, dst);
	}

	return bartxt;
}

///

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
	struct Data *data;
	Object *folderString;
	Object *folderInfoStr;
	Object *statusGroup;
	Object *gauge;
	Object *gaugeLabel;
	Object *infoText;

	if (!(obj = DoSuperNew(cl, obj,

		TextFrame,
		MUIA_Background,    MUII_TextBack,
		MUIA_Group_Horiz,   TRUE,
		MUIA_InnerTop,      2,
		MUIA_InnerBottom,   2,
		MUIA_InnerLeft,     2,
		MUIA_InnerRight,    2,

		Child, HGroup,
			Child, folderString = TextObject,
				MUIA_HorizWeight,   0,
				MUIA_Font,          MUIV_Font_Big,
				MUIA_Text_PreParse, "\033b",
			End,

			Child, folderInfoStr = TextObject,
				MUIA_Text_PreParse, "\033l",
			End,
	    End,

		Child, gaugeLabel = TextObject,
			MUIA_Text_PreParse, "\033r",
		End,

		Child, statusGroup = PageGroup,
			Child, HSpace(0),
			Child, gauge = GaugeObject,
				GaugeFrame,
				MUIA_Gauge_Horiz,    TRUE,
				MUIA_Gauge_InfoText, " ",
			End,
			Child, infoText = TextObject,
				MUIA_Text_PreParse, "\033r",
			End,
		End,

		TAG_MORE, inittags(msg))))

	return 0;

	data = (struct Data *)INST_DATA(cl,obj);

	data->TX_FOLDER = folderString;
	data->TX_FINFO  = folderInfoStr;
	data->GA_GROUP  = statusGroup;
	data->GA_LABEL  = gaugeLabel;
	data->GA_INFO   = gauge;
	data->TX_INFO   = infoText;

	return (ULONG)obj;
}
///

/* Public Methods */
/// DECLARE(SetFolder)
/* set a new folder and update its name and image in the infobar */
DECLARE(SetFolder) // struct Folder *newFolder
{
	GETDATA;

	struct Folder *folder = msg->newFolder;
	struct BodyChunkData *bcd = NULL;

	data->actualFolder = folder;

	if(!folder) return NULL;

	// set the name of the folder as the info text
	set(data->TX_FOLDER, MUIA_Text_Contents, folder->Name);

	// now we are going to set some status field at the right side of the folder name
	set(data->TX_FINFO, MUIA_Text_Contents, GetFolderInfo(folder));

	// Prepare the GR_INFO group for adding a new child
	if(DoMethod(obj, MUIM_Group_InitChange))
	{
		if(data->BC_INFO)
		{
			DoMethod(obj, OM_REMMEMBER, data->BC_INFO);
			MUI_DisposeObject(data->BC_INFO);
			data->BC_INFO = NULL;
		}

		if(folder->FImage)
			bcd = folder->FImage;
		else if(folder->ImageIndex >= 0)
			bcd = G->BImage[folder->ImageIndex+(MAXIMAGES-MAXBCSTDIMAGES)];

		if(bcd)
		{
			data->BC_INFO = BodychunkObject,
				MUIA_FixWidth,             bcd->Width,
				MUIA_FixHeight,            bcd->Height,
				MUIA_Bitmap_Width,         bcd->Width,
				MUIA_Bitmap_Height,        bcd->Height,
				MUIA_Bitmap_SourceColors,  bcd->Colors,
				MUIA_Bodychunk_Depth,      bcd->Depth,
				MUIA_Bodychunk_Body,       bcd->Body,
				MUIA_Bodychunk_Compression,bcd->Compression,
				MUIA_Bodychunk_Masking,    bcd->Masking,
				MUIA_Bitmap_Transparent,   0,
				MUIA_InnerBottom,          0,
				MUIA_InnerLeft,            0,
				MUIA_InnerRight,           0,
				MUIA_InnerTop,             0,
			End;

			if(data->BC_INFO) DoMethod(obj, OM_ADDMEMBER, data->BC_INFO);
		}

		DoMethod(obj, MUIM_Group_ExitChange);
	}
}
///
/// DECLARE(RefreshText)
/* refreshes the text at the right of the folder name */
DECLARE(RefreshText)
{
	GETDATA;

	// set the name of the folder as the info text
	set(data->TX_FOLDER, MUIA_Text_Contents, data->actualFolder->Name);

	// now we are going to set some status field at the right side of the folder name
	set(data->TX_FINFO, MUIA_Text_Contents, GetFolderInfo(data->actualFolder));

	return 0;
}
///
/// DECLARE(ShowGauge)
/* activates the gauge in the InfoBar with the passed text and percentage */
DECLARE(ShowGauge) // STRPTR gaugeText, LONG perc, LONG max
{
	GETDATA;

	static char infoText[256];

	if(msg->gaugeText != NULL)
	{
		set(data->GA_LABEL, MUIA_Text_Contents, msg->gaugeText);

		sprintf(infoText, "%%ld/%ld", msg->max);

		SetAttrs(data->GA_INFO,
			MUIA_Gauge_InfoText,  infoText,
			MUIA_Gauge_Current,   msg->perc,
			MUIA_Gauge_Max,       msg->max
		);

		set(data->GA_GROUP, MUIA_Group_ActivePage, 1);
	}
	else
	{
		set(data->GA_INFO, MUIA_Gauge_Current, msg->perc);
		set(data->GA_GROUP, MUIA_Group_ActivePage, 1);
	}

	return TRUE;
}
///
/// DECLARE(ShowInfoText)
/* activates the gauge in the InfoBar with the passed text and percentage */
DECLARE(ShowInfoText) // STRPTR infoText
{
	GETDATA;

	set(data->GA_GROUP, MUIA_Group_ActivePage, 2);

	if(msg->infoText != NULL)
	{
		set(data->TX_INFO, MUIA_Text_Contents, msg->infoText);
	}
	else
	{
		set(data->TX_INFO, MUIA_Text_Contents, " ");
	}

	return TRUE;
}
///
/// DECLARE(HideBars)
/* activates the gauge in the InfoBar with the passed text and percentage */
DECLARE(HideBars)
{
	GETDATA;

	set(data->GA_GROUP, MUIA_Group_ActivePage, 0);
	set(data->GA_LABEL, MUIA_Text_Contents, " ");

	return TRUE;
}
///
