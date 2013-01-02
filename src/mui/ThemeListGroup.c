/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2013 YAM Open Source Team

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
 YAM OpenSource project    : http://sourceforge.net/projects/yamos/

 $Id$

 Superclass:  MUIC_Group
 Description: Provides GUI elements and routines for showing the currently
              available themes for YAM.

***************************************************************************/

#include "ThemeListGroup_cl.h"

#include <string.h>
#include <proto/dos.h>
#include <proto/muimaster.h>
#include <mui/NBalance_mcc.h>
#include <mui/NList_mcc.h>
#include <mui/NListview_mcc.h>

#include "YAM.h"
#include "YAM_config.h"

#include "Locale.h"
#include "MUIObjects.h"
#include "Requesters.h"

#include "mui/ImageArea.h"
#include "mui/ThemeList.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *NL_THEMELIST;
  Object *TX_THEMELABEL;
  Object *GR_PREVIEW;
  Object *IM_PREVIEW;
  Object *BT_ACTIVATE;
  Object *TX_AUTHOR;
  Object *TX_URL;
  char themeName[SIZE_DEFAULT];
};
*/

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  Object *themeListObject;
  Object *themeTextObject;
  Object *previewImageObject;
  Object *imageGroupObject;
  Object *activateButtonObject;
  Object *authorTextObject;
  Object *urlTextObject;

  ENTER();

  if((obj = DoSuperNew(cl, obj,
          GroupSpacing(0),
          MUIA_Group_Horiz,       TRUE,
          MUIA_ContextMenu,       FALSE,

          Child, VGroup,
            MUIA_HorizWeight, 30,

            Child, HBarT(tr(MSG_CO_LOOKFEEL_TITLE_THEME)), End,

            Child, NListviewObject,
              MUIA_CycleChain, TRUE,
              MUIA_Weight, 60,
              MUIA_NListview_NList, themeListObject = ThemeListObject,
              End,
            End,

            Child, activateButtonObject = MakeButton(tr(MSG_CO_THEME_ACTIVATE)),
          End,

          Child, NBalanceObject,
            MUIA_Balance_Quiet, TRUE,
          End,

          Child, VGroup,
            GroupFrameT(tr(MSG_CO_LOOKFEEL_TITLE_INFO)),
            MUIA_HorizWeight, 70,

            Child, themeTextObject = TextObject,
              MUIA_Text_PreParse, "\033b\033c",
              MUIA_Text_Copy,     FALSE,
            End,

            Child, RectangleObject,
              MUIA_Rectangle_HBar, TRUE,
              MUIA_FixHeight,      4,
            End,

            Child, HGroup,
              Child, HSpace(0),
              Child, TextObject,
                MUIA_Font,          MUIV_Font_Tiny,
                MUIA_HorizWeight,   0,
                MUIA_Text_Contents, tr(MSG_CO_THEME_PREVIEW),
                MUIA_Text_Copy,     FALSE,
              End,
              Child, HSpace(0),
            End,

            Child, HVSpace,

            Child, HGroup,
              Child, HSpace(0),

              Child, imageGroupObject = VGroup,
                Child, previewImageObject = ImageAreaObject,
                  MUIA_ImageArea_ShowLabel,   FALSE,
                  MUIA_ImageArea_MaxWidth,    300,
                  MUIA_ImageArea_MaxHeight,   200,
                  MUIA_ImageArea_NoMinHeight, FALSE,
                End,
              End,

              Child, HSpace(0),
            End,

            Child, HVSpace,

            Child, RectangleObject,
              MUIA_Rectangle_HBar, TRUE,
              MUIA_FixHeight,      4,
            End,

            Child, ColGroup(2),
              Child, Label2(tr(MSG_CO_THEME_AUTHOR)),
              Child, authorTextObject = TextObject,
                TextFrame,
                MUIA_Background,  MUII_TextBack,
                MUIA_Text_SetMin, TRUE,
                MUIA_Text_Copy,   FALSE,
              End,

              Child, Label2(tr(MSG_CO_THEME_URL)),
              Child, urlTextObject = TextObject,
                TextFrame,
                MUIA_Background,  MUII_TextBack,
                MUIA_Text_SetMin, TRUE,
                MUIA_Text_Copy,   FALSE,
              End,
            End,

          End,

        TAG_MORE, inittags(msg))) != NULL)
  {
    GETDATA;

    data->NL_THEMELIST = themeListObject;
    data->TX_THEMELABEL = themeTextObject;
    data->GR_PREVIEW = imageGroupObject;
    data->IM_PREVIEW = previewImageObject;
    data->BT_ACTIVATE = activateButtonObject;
    data->TX_AUTHOR = authorTextObject;
    data->TX_URL = urlTextObject;

    // set notifies
    DoMethod(themeListObject,      MUIM_Notify, MUIA_NList_SelectChange, TRUE, obj, 1, METHOD(SelectionChanged));
    DoMethod(themeListObject,      MUIM_Notify, MUIA_NList_DoubleClick, MUIV_EveryTime, obj, 1, METHOD(ActivateTheme));
    DoMethod(activateButtonObject, MUIM_Notify, MUIA_Pressed, FALSE, obj, 1, METHOD(ActivateTheme));
  }

  RETURN((IPTR)obj);
  return (IPTR)obj;
}

///

/* Public Methods */
/// DECLARE(Update)
DECLARE(Update)
{
  GETDATA;
  BOOL result = FALSE;
  APTR context;

  ENTER();

  // clear the NList
  DoMethod(data->NL_THEMELIST, MUIM_NList_Clear);

  // prepare for an ExamineDir()
  if((context = ObtainDirContextTags(EX_StringName, (ULONG)G->ThemesDir,
                                     EX_DataFields, EXF_TYPE|EXF_NAME,
                                     TAG_DONE)) != NULL)
  {
    struct ExamineData *ed;
    LONG error;

    // iterate through the entries of the Themes directory
    while((ed = ExamineDir(context)) != NULL)
    {
      // check that this entry is a drawer
      // because we don't accept any file here
      if(EXD_IS_DIRECTORY(ed))
      {
        struct Theme theme;
        char filename[SIZE_PATHFILE];

        D(DBF_CONFIG, "found dir '%s' in themes drawer", ed->Name);

        // clear our temporary themes structure
        memset(&theme, 0, sizeof(struct Theme));

        // now we check whether this is a drawer which contains a
        // ".theme" file which should be a sign that this is a YAM theme
        AddPath(theme.directory, G->ThemesDir, ed->Name, sizeof(theme.directory));
        AddPath(filename, theme.directory, ".theme", sizeof(filename));

        // parse the .theme file to check wheter this
        // is a valid theme or not.
        if(ParseThemeFile(filename, &theme) > 0)
        {
          D(DBF_CONFIG, "found valid .theme file '%s'", filename);

          // add the theme to our NList which in fact will allocate/free everything the
          // ParseThemeFile() function did allocate previously.
          DoMethod(data->NL_THEMELIST, MUIM_NList_InsertSingle, &theme, MUIV_NList_Insert_Sorted);

          result = TRUE;
        }
        else
        {
          W(DBF_CONFIG, "couldn't parse .theme file '%s'", filename);
          FreeTheme(&theme);
        }
      }
      else
        W(DBF_CONFIG, "unknown file '%s' in themes directory ignored", ed->Name);
    }

    error = IoErr();
    if(error != 0 && error != ERROR_NO_MORE_ENTRIES)
      E(DBF_CONFIG, "ExamineDir() failed, error %ld", error);

    // now we have to check which item we should set active
    if(xget(data->NL_THEMELIST, MUIA_NList_Entries) > 1)
    {
      // walk through our list and check if the theme is the currently
      // active one, and if so we go and make it the currently selected one.
      ULONG pos;
      BOOL found = FALSE;
      for(pos=0;;pos++)
      {
        struct Theme *theme = NULL;

        DoMethod(data->NL_THEMELIST, MUIM_NList_GetEntry, pos, &theme);
        if(theme == NULL)
          break;

        if(stricmp(FilePart(theme->directory), CE->ThemeName) == 0)
        {
          set(data->NL_THEMELIST, MUIA_NList_Active, pos);
          found = TRUE;
          break;
        }
      }

      if(found == FALSE)
        set(data->NL_THEMELIST, MUIA_NList_Active, MUIV_NList_Active_Top);
    }
    else
      set(data->NL_THEMELIST, MUIA_NList_Active, MUIV_NList_Active_Top);

    ReleaseDirContext(context);
  }
  else
    E(DBF_CONFIG, "No themes directory found!");

  RETURN(result);
  return result;
}

///
/// DECLARE(SelectionChanged)
DECLARE(SelectionChanged)
{
  GETDATA;
  struct Theme *theme = NULL;

  ENTER();

  // get the currently selected entry
  DoMethod(data->NL_THEMELIST, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &theme);
  if(theme != NULL)
  {
    if(DoMethod(data->GR_PREVIEW, MUIM_Group_InitChange))
    {
      char filename[SIZE_PATHFILE];

      AddPath(filename, theme->directory, "preview", sizeof(filename));

      // set the new attributes, the old image will be deleted from the cache
      xset(data->IM_PREVIEW, MUIA_ImageArea_ID,       filename,
                             MUIA_ImageArea_Filename, filename);

      // and force a cleanup/setup pair
      DoMethod(data->GR_PREVIEW, OM_REMMEMBER, data->IM_PREVIEW);
      DoMethod(data->GR_PREVIEW, OM_ADDMEMBER, data->IM_PREVIEW);

      DoMethod(data->GR_PREVIEW, MUIM_Group_ExitChange);
    }

    snprintf(data->themeName, sizeof(data->themeName), "%s - %s", theme->name, theme->version);
    set(data->TX_THEMELABEL, MUIA_Text_Contents, data->themeName);
    set(data->TX_AUTHOR, MUIA_Text_Contents, theme->author);
    set(data->TX_URL, MUIA_Text_Contents, theme->url);
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(ActivateTheme)
DECLARE(ActivateTheme)
{
  GETDATA;
  struct Theme *theme = NULL;

  ENTER();

  // get the currently selected entry
  DoMethod(data->NL_THEMELIST, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &theme);
  if(theme != NULL)
  {
    char *themeName = FilePart(theme->directory);

    // check that this theme isn't already the
    // active one.
    if(stricmp(themeName, CE->ThemeName) != 0)
    {
      // now we activate the theme and we warn the user about
      // the fact that a restart is required for the new theme to
      // be activated.
      strlcpy(CE->ThemeName, themeName, sizeof(CE->ThemeName));

      // redraw the NList.
      DoMethod(data->NL_THEMELIST, MUIM_NList_Redraw, MUIV_NList_Redraw_All);

      // remind the users to save the configuration and
      // restart yam.
      MUI_Request(G->App, _win(obj), MUIF_NONE, tr(MSG_ER_THEME_ACTIVATED_TITLE),
                                                tr(MSG_OkayReq),
                                                tr(MSG_ER_THEME_ACTIVATED));
    }
    else
      W(DBF_THEME, "theme '%s' is already the currently active one!", themeName);
  }

  RETURN(0);
  return 0;
}

///
