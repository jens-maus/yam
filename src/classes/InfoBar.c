/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2006 by YAM Open Source Team

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
 YAM OpenSource project     : http://sourceforge.net/projects/yamos/

 $Id$

 Superclass:  MUIC_Group
 Description: Displays additional information in the Main window

***************************************************************************/

#include "InfoBar_cl.h"

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
  struct BodyChunkData *actualImage;
  struct Folder *actualFolder;
  struct timeval last_gaugemove;
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
  bartxt[0] = '\0';

  // Lets create the label of the AppIcon now
  for (src = C->InfoBarText; *src; src++)
  {
    if (*src == '%')
    {
      switch (*++src)
      {
        case '%': strcpy(dst, "%");                    break;
        case 'n': sprintf(dst, "%d", folder->New);     break;
        case 'u': sprintf(dst, "%d", folder->Unread);  break;
        case 't': sprintf(dst, "%d", folder->Total);   break;
        case 's': sprintf(dst, "%d", folder->Sent);    break;
        case 'd': sprintf(dst, "%d", folder->Deleted); break;
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
    Child, HGroup,
      InnerSpacing(0,0),
      Child, folderString = TextObject,
        MUIA_HorizWeight,   0,
        MUIA_Font,          MUIV_Font_Big,
        MUIA_Text_SetMax,   FALSE,
        MUIA_Text_PreParse, "\033b",
      End,
      Child, folderInfoStr = TextObject,
        MUIA_Font,          MUIV_Font_Tiny,
        MUIA_Text_SetMax,   FALSE,
        MUIA_Text_PreParse, "\033l",
      End,
    End,
      
    Child, gaugeLabel = TextObject,
      MUIA_Text_SetMax,   FALSE,
      MUIA_Text_PreParse, "\033r",
    End,

    Child, statusGroup = PageGroup,
      Child, HSpace(0),
      Child, gauge = GaugeObject,
        GaugeFrame,
        MUIA_Gauge_Horiz,    TRUE,
        MUIA_Gauge_InfoText, "",
      End,
      Child, infoText = TextObject,
        MUIA_Text_SetMax,   FALSE,
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

  if(!folder) return -1;

  // prepare the object for a change
  if(DoMethod(obj, MUIM_Group_InitChange))
  {
    // set the name of the folder as the info text
    nnset(data->TX_FOLDER, MUIA_Text_Contents, folder->Name);

    // now we are going to set some status field at the right side of the folder name
    nnset(data->TX_FINFO, MUIA_Text_Contents, GetFolderInfo(folder));

    // lets see which image we should display
    if(folder->FImage)                 bcd = folder->FImage;
    else if(folder->ImageIndex >= 0)  bcd = G->BImage[folder->ImageIndex+MAXBCSTATUSIMG];
    else bcd = NULL;

    // only if the image should be changed we proceed or otherwise
    // MUI will refresh too often
    if(data->actualImage != bcd)
    {
      // now we check whether we have to remove the old bodychunkobject first
      if(data->BC_INFO)
      {
        DoMethod(obj, OM_REMMEMBER, data->BC_INFO);
        MUI_DisposeObject(data->BC_INFO);
        data->BC_INFO = NULL;
      }

      // and if we have a new one we generate the object an add it
      // to the grouplist of this infobar
      if(bcd)
      {
        data->BC_INFO = BodychunkObject,
          InnerSpacing(0,0),
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
        End;

        if(data->BC_INFO)
        {
          DoMethod(obj, OM_ADDMEMBER, data->BC_INFO);
        }
      }
    }

    // now that we are finished we can call ExitChange to refresh the infobar
    DoMethod(obj, MUIM_Group_ExitChange);
  }

  // and lets point the instancedata to our new image
  // so that we can check later if we should really update it or not.
  data->actualImage = bcd;

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
    nnset(data->GA_LABEL, MUIA_Text_Contents, msg->gaugeText);

    sprintf(infoText, "%%ld/%ld", msg->max);

    SetAttrs(data->GA_INFO,
      MUIA_Gauge_InfoText,  infoText,
      MUIA_Gauge_Current,   msg->perc,
      MUIA_Gauge_Max,       msg->max,
    TAG_DONE);

    set(data->GA_GROUP, MUIA_Group_ActivePage, 1);
  }
  else
  {
    struct timeval now;

    // then we update the gauge, but we take also care of not refreshing
    // it too often or otherwise it slows down the whole search process.
    GetSysTime(&now);
    if(-CmpTime(&now, &data->last_gaugemove) > 0)
    {
      struct timeval delta;

      // how much time has passed exactly?
      memcpy(&delta, &now, sizeof(struct timeval));
      SubTime(&delta, &data->last_gaugemove);

      // update the display at least twice a second
      if(delta.tv_secs > 0 || delta.tv_micro > 250000)
      {
        set(data->GA_INFO, MUIA_Gauge_Current, msg->perc);
        memcpy(&data->last_gaugemove, &now, sizeof(struct timeval));
      }
    }
    
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

  nnset(data->GA_GROUP, MUIA_Group_ActivePage, 2);

  if(msg->infoText != NULL)
  {
    nnset(data->TX_INFO, MUIA_Text_Contents, msg->infoText);
  }
  else
  {
    nnset(data->TX_INFO, MUIA_Text_Contents, "");
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
