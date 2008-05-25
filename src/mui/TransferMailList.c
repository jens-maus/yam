/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2008 by YAM Open Source Team

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

 Superclass:  MUIC_NList
 Description: NList class of the transfer window

***************************************************************************/

#include "TransferMailList_cl.h"

#include "Themes.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *downloadImage;
  Object *deleteImage;
};
*/

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  struct Data *data;

  ENTER();

  if(!(obj = DoSuperNew(cl, obj,
    TAG_MORE, inittags(msg))))
  {
    RETURN(0);
    return 0;
  }

  data = (struct Data *)INST_DATA(cl,obj);

  // prepare the group image
  data->downloadImage = MakeImageObject("status_download", G->theme.statusImages[si_Download]);
  data->deleteImage   = MakeImageObject("status_delete", G->theme.statusImages[si_Delete]);

  DoMethod(obj, MUIM_NList_UseImage, data->downloadImage, si_Download, MUIF_NONE);
  DoMethod(obj, MUIM_NList_UseImage, data->deleteImage, si_Delete, MUIF_NONE);

  RETURN((ULONG)obj);
  return (ULONG)obj;
}

///
/// OVERLOAD(OM_DISPOSE)
OVERLOAD(OM_DISPOSE)
{
  GETDATA;

  if(data->downloadImage != NULL)
  {
    DoMethod(obj, MUIM_NList_UseImage, NULL, si_Download, MUIF_NONE);
    MUI_DisposeObject(data->downloadImage);
    data->downloadImage = NULL;
  }

  if(data->deleteImage != NULL)
  {
    DoMethod(obj, MUIM_NList_UseImage, NULL, si_Delete, MUIF_NONE);
    MUI_DisposeObject(data->deleteImage);
    data->deleteImage = NULL;
  }

  return DoSuperMethodA(cl,obj,msg);
}

///

/* Private Functions */

/* Public Methods */
