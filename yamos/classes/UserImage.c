/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2005 by YAM Open Source Team

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

 Superclass:  MUIC_Area
 Description: Custom class to load&display a user image in readmailgroup
							object.

***************************************************************************/

#include "UserImage_cl.h"

#include <proto/datatypes.h>
#include <proto/icon.h>
#include <proto/graphics.h>

/* CLASSDATA
struct Data
{
	char fileName[SIZE_PATHFILE];
	Object *datatypeObject;
	struct BitMapHeader *bitMapHeader;
	struct BitMap *bitMap;

	int maxHeight;
	int maxWidth;
};
*/

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
	if((obj = DoSuperNew(cl, obj,
			MUIA_FillArea, FALSE, // do not care about background filling
		TAG_MORE, inittags(msg))))
	{
		GETDATA;
		struct TagItem *tags = inittags(msg);
		struct TagItem *tag;

		while((tag = NextTagItem(&tags)))
		{
			switch(tag->ti_Tag)
			{
				ATTR(File) 			: if(tag->ti_Data) stccpy(data->fileName, (char *)tag->ti_Data, SIZE_PATHFILE); break;
				ATTR(MaxHeight) : data->maxHeight = (BOOL)tag->ti_Data ; break;
				ATTR(MaxWidth)  : data->maxWidth  = (BOOL)tag->ti_Data ; break;
			}
		}
	}

	return (ULONG)obj;
}
///
/// OVERLOAD(MUIM_Setup)
OVERLOAD(MUIM_Setup)
{
	GETDATA;
	struct Process *myproc;
	APTR oldWindowPtr;

	// call the SuperMethod() first
	if(!DoSuperMethodA(cl,obj,msg) ||
		 data->fileName[0] == '\0')
		return FALSE;

	// tell DOS not to bother us with requesters
	myproc = (struct Process *)FindTask(NULL);
	oldWindowPtr = myproc->pr_WindowPtr;
	myproc->pr_WindowPtr = (APTR)-1;

	// create the datatypes object
	data->datatypeObject = NewDTObject(data->fileName,
		DTA_SourceType,					DTST_FILE,
		DTA_GroupID,						GID_PICTURE,
		OBP_Precision, 					PRECISION_IMAGE,
		PDTA_Remap,							TRUE,
		PDTA_DestMode, 					PMODE_V43,
		PDTA_Screen, 						(ULONG)_screen(obj),
		PDTA_FreeSourceBitMap, 	TRUE,
		PDTA_UseFriendBitMap, 	TRUE,
		PDTA_ScaleQuality,			0,
	TAG_DONE);

	// restore the window pointer of DOS again.
	myproc->pr_WindowPtr = oldWindowPtr;

	// if we had success in loading the datatypes object we can go on
	if(data->datatypeObject)
	{
		struct FrameInfo fri;

		memset(&fri, 0, sizeof(struct FrameInfo));

		// get some information about the image from picture.datatype
		DoMethod(data->datatypeObject, DTM_FRAMEBOX, NULL, (ULONG)&fri, (ULONG)&fri, sizeof(struct FrameInfo), 0);

		// check if the datatype object is somewhat valid
		if(fri.fri_Dimensions.Depth > 0)
		{
			// make sure to scale down the image if maxHeight/maxWidth is specified
			LONG scaleHeightDiff = fri.fri_Dimensions.Height - data->maxHeight;
			LONG scaleWidthDiff  = fri.fri_Dimensions.Width - data->maxWidth;

			if(scaleHeightDiff > 0 || scaleWidthDiff > 0)
			{
				// make sure we are scaling proportional
				LONG scaleFactor = MAX(scaleHeightDiff, scaleWidthDiff);
				LONG newHeight = fri.fri_Dimensions.Height - scaleFactor;
				LONG newWidth = fri.fri_Dimensions.Width - scaleFactor;

				// scale it now
				DB(kprintf("UserImage scale (h/w) from %ld/%ld to %ld/%ld\n", fri.fri_Dimensions.Height,
																																			fri.fri_Dimensions.Width,
																																			newHeight,
																																			newWidth);)
				DoMethod(data->datatypeObject, PDTM_SCALE,
																			 newWidth  > 0 ? newWidth  : 1,
																			 newHeight > 0 ? newHeight : 1,
																			 0);
			}

			// remap the BitMap with DTM_PROCLAYOUT
			if(DoMethod(data->datatypeObject, DTM_PROCLAYOUT, NULL, 1))
			{
				data->bitMapHeader = (struct BitMapHeader *)xget(data->datatypeObject, PDTA_BitMapHeader);

				if(data->bitMapHeader)
				{
					GetDTAttrs(data->datatypeObject, PDTA_DestBitMap, (ULONG)&data->bitMap, TAG_DONE);
				
					if(!data->bitMap)
						GetDTAttrs(data->datatypeObject, PDTA_BitMap, (ULONG)&data->bitMap, TAG_DONE);

					if(data->bitMap)
						return TRUE;
				}
			}
		}

		DisposeDTObject(data->datatypeObject);
		data->datatypeObject = NULL;
	}

	// if we reach here, the something must have gone wrong
	data->bitMap = NULL;
	data->bitMapHeader = NULL;

	return FALSE;
}
///
/// OVERLOAD(MUIM_Cleanup)
OVERLOAD(MUIM_Cleanup)
{
	GETDATA;

	data->bitMap = NULL;
	data->bitMapHeader = NULL;

	if(data->datatypeObject)
	{
		DisposeDTObject(data->datatypeObject);
		data->datatypeObject = NULL;
	}

	return DoSuperMethodA(cl, obj, msg);
}
///
/// OVERLOAD(MUIM_Draw)
OVERLOAD(MUIM_Draw)
{
	GETDATA;

	// call the SuperMethod
	DoSuperMethodA(cl, obj, msg);

	if(((struct MUIP_Draw *)msg)->flags & MADF_DRAWOBJECT)
	{
		if(data->bitMap)
			BltBitMapRastPort(data->bitMap, 0, 0, _rp(obj), _mleft(obj), _mtop(obj), _mwidth(obj), _mheight(obj), 0xc0);
	}

	return 0;
}
///
/// OVERLOAD(MUIM_AskMinMax)
OVERLOAD(MUIM_AskMinMax)
{
	GETDATA;
	struct MUI_MinMax *mi;

	// call the SuperMethod
	DoSuperMethodA(cl, obj, msg);

	mi = ((struct MUIP_AskMinMax *)msg)->MinMaxInfo;

	if(data->bitMap &&
		 data->bitMapHeader)
	{
		mi->MinWidth  += data->bitMapHeader->bmh_Width ;
		mi->MinHeight += data->bitMapHeader->bmh_Height;
		mi->DefWidth  += data->bitMapHeader->bmh_Width ;
		mi->DefHeight += data->bitMapHeader->bmh_Height;
		mi->MaxWidth  += data->bitMapHeader->bmh_Width ;
		mi->MaxHeight += data->bitMapHeader->bmh_Height;
	}

	return 0;
}
///

/* Private Functions */

/* Public Methods */
