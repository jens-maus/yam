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
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 YAM Official Support Site : http://www.yam.ch
 YAM OpenSource project		 : http://sourceforge.net/projects/yamos/

 $Id$

 Superclass:  MUIC_Bodychunk
 Description: Auto-completes email addresses etc.

***************************************************************************/

#include "BodychunkImage_cl.h"

/* CLASSDATA
struct Data
{
	struct BodyChunkData *BCD;
	BOOL useCached;
};
*/

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
	if((obj = DoSuperNew(cl, obj,
		MUIA_FixWidth, 		16,
		MUIA_FixHeight, 	16,
		MUIA_InnerBottom, 0,
		MUIA_InnerLeft, 	0,
		MUIA_InnerRight, 	0,
		MUIA_InnerTop,		0,
		TAG_MORE, inittags(msg))))
	{
		GETDATA;
		char fname[SIZE_PATHFILE];
		struct TagItem *tags = inittags(msg);
		struct TagItem *tag;

		*fname = '\0';

		while((tag = NextTagItem(&tags)))
		{
			switch(tag->ti_Tag)
			{
				ATTR(File) 			: if(tag->ti_Data) stccpy(fname, (char *)tag->ti_Data, SIZE_PATHFILE); break;
				ATTR(UseCached) : data->useCached = (BOOL)tag->ti_Data ; break;
			}
		}

		// now that we have set all attributes we go and
		// load the bodychunk data from the file specified
		if(*fname)
		{
			if(data->useCached)
				data->BCD = GetBCImage(fname);
			else
				data->BCD = LoadBCImage(fname);

			if(data->BCD)
			{
				SetAttrs(obj,
									MUIA_FixWidth,              data->BCD->Width,
									MUIA_FixHeight,             data->BCD->Height,
									MUIA_Bitmap_Width,          data->BCD->Width,
									MUIA_Bitmap_Height,         data->BCD->Height,
									MUIA_Bitmap_SourceColors,   data->BCD->Colors,
									MUIA_Bodychunk_Depth,       data->BCD->Depth,
									MUIA_Bodychunk_Body,        data->BCD->Body,
									MUIA_Bodychunk_Compression, data->BCD->Compression,
									MUIA_Bodychunk_Masking,     data->BCD->Masking,
								TAG_DONE);
			}
		}
	}

	return (ULONG)obj;
}
///
/// OVERLOAD(OM_DISPOSE)
OVERLOAD(OM_DISPOSE)
{
	GETDATA;

	if(!data->useCached && data->BCD)
		FreeBCImage(data->BCD);
	
	return DoSuperMethodA(cl, obj, msg);
}
///

/* Private Functions */

/* Public Methods */
