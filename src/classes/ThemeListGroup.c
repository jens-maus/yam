/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2007 by YAM Open Source Team

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

#include "extrasrc.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *NL_THEMELIST;
  Object *BT_ACTIVATE;
  Object *TX_AUTHOR;
  Object *TX_URL;

  char activeTheme[SIZE_FILE];
};
*/

/* Hooks */
/// ConstructHook
HOOKPROTONHNO(ConstructFunc, struct Theme *, struct Theme *e)
{
  struct Theme *entry;

  ENTER();

  entry = memdup(e, sizeof(*e));

  RETURN(entry);
  return entry;
}
MakeStaticHook(ConstructHook, ConstructFunc);

///
/// DestructHook
//  destruction hook
HOOKPROTONHNO(DestructFunc, LONG, struct Theme *entry)
{
  FreeTheme(entry);
  return 0;
}
MakeStaticHook(DestructHook, DestructFunc);

///
/// DisplayHook
HOOKPROTONHNO(DisplayFunc, LONG, struct NList_DisplayMessage *msg)
{
  struct Theme *entry;
  char **array;

  if(!msg)
    return 0;

  // now we set our local variables to the DisplayMessage structure ones
  entry = (struct Theme *)msg->entry;
  array = msg->strings;

  if(entry)
  {
    array[0] = entry->name;
  }

  return 0;
}
MakeStaticHook(DisplayHook, DisplayFunc);

///

/* Private Functions */

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  Object *themeListObject;
  Object *activateButtonObject;
  Object *authorTextObject;
  Object *urlTextObject;

  ENTER();

  obj = DoSuperNew(cl, obj,
          MUIA_Group_Horiz,       TRUE,
          MUIA_ContextMenu,       FALSE,

          Child, VGroup,
            MUIA_HorizWeight, 30,
            Child, NListviewObject,
              MUIA_CycleChain, TRUE,
              MUIA_NListview_NList, themeListObject = NListObject,
                InputListFrame,
                MUIA_NList_DragType,      MUIV_NList_DragType_None,
                MUIA_NList_ConstructHook, &ConstructHook,
                MUIA_NList_DestructHook,  &DestructHook,
                MUIA_NList_DisplayHook2,  &DisplayHook,
              End,
            End,

            Child, activateButtonObject = MakeButton(tr(MSG_CO_THEME_ACTIVATE)),
          End,

          Child, BalanceObject, End,

          Child, VGroup,
            MUIA_HorizWeight, 70,
            Child, HGroup,
              Child, HSpace(0),
              Child, TextObject,
                MUIA_Text_Contents, tr(MSG_CO_THEME_PREVIEW),
                MUIA_Font,          MUIV_Font_Tiny,
                MUIA_HorizWeight,   0,
              End,
              Child, HSpace(0),
            End,

            Child, RectangleObject,
              MUIA_Rectangle_HBar, TRUE,
              MUIA_FixHeight,      4,
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
              End,

              Child, Label2(tr(MSG_CO_THEME_URL)),
              Child, urlTextObject = TextObject,
                TextFrame,
                MUIA_Background,  MUII_TextBack,
                MUIA_Text_SetMin, TRUE,
              End,
            End,
          End,

        TAG_MORE, inittags(msg));

  if(obj != NULL)
  {
    GETDATA;

    data->NL_THEMELIST = themeListObject;
    data->BT_ACTIVATE = activateButtonObject;
    data->TX_AUTHOR = authorTextObject;
    data->TX_URL = urlTextObject;
  }

  RETURN((ULONG)obj);
  return (ULONG)obj;
}
///
/// OVERLOAD(OM_GET)
/* this is just so that we can notify the popup tag */
OVERLOAD(OM_GET)
{
  GETDATA;
  ULONG *store = ((struct opGet *)msg)->opg_Storage;

  switch(((struct opGet *)msg)->opg_AttrID)
  {
    ATTR(Active) : *store = (ULONG)data->activeTheme; return TRUE;
  }

  return DoSuperMethodA(cl, obj, msg);
}
///
/// OVERLOAD(OM_SET)
OVERLOAD(OM_SET)
{
  GETDATA;

  struct TagItem *tags = inittags(msg), *tag;
  while((tag = NextTagItem(&tags)))
  {
    switch(tag->ti_Tag)
    {
      ATTR(Active):
      {
        strlcpy(data->activeTheme, (char*)tag->ti_Data, sizeof(data->activeTheme));

        // make the superMethod call ignore those tags
        tag->ti_Tag = TAG_IGNORE;
      }
      break;
    }
  }

  return DoSuperMethodA(cl, obj, msg);
}
///

/* Public Methods */
/// DECLARE(Update)
DECLARE(Update)
{
  GETDATA;
  BOOL result = FALSE;
  char themesDir[SIZE_PATH];
  APTR context;

  ENTER();

  // construct the themes directory path
  AddPath(themesDir, G->ProgDir, "Themes", sizeof(themesDir));

  // prepare for an ExamineDir()
  if((context = ObtainDirContextTags(EX_StringName, themesDir, TAG_DONE)) != NULL)
  {
    struct ExamineData *ed;

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
        AddPath(filename, themesDir, ed->Name, sizeof(filename));
        AddPart(filename, ".theme", sizeof(filename));

        // parse the .theme file to check wheter this
        // is a valid theme or not.
        if(ParseThemeFile(filename, &theme, TRUE) > 0)
        {
          D(DBF_CONFIG, "found valid .theme file '%s'", filename);

          // add the theme to our NList which in fact will allocate/free everything the
          // ParseThemeFile() function did allocate previously.
          DoMethod(data->NL_THEMELIST, MUIM_NList_InsertSingle, &theme, MUIV_NList_Insert_Bottom);

          result = TRUE;
        }
        else
          W(DBF_CONFIG, "couldn't parse .theme file '%s'", filename);
      }
      else
        W(DBF_CONFIG, "unknown file '%s' in themes directory ignored", ed->Name);
    }

    if(IoErr() != ERROR_NO_MORE_ENTRIES)
      E(DBF_CONFIG, "ExamineDir() failed");

    ReleaseDirContext(context);
  }
  else
    E(DBF_CONFIG, "No themes directory found!");

  RETURN(result);
  return result;
}
///

