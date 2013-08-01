
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

#include <stdlib.h>
#include <proto/muimaster.h>
#include <mui/Crawling_mcc.h>
#include <mui/NFloattext_mcc.h>
#include <mui/Urltext_mcc.h>

#include "YAM.h"
#include "YAM_global.h"
#include "YAM_stringsizes.h"

#include "Locale.h"
#include "MUIObjects.h"

#include "mui/ImageArea.h"
#include "mui/YAMApplication.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  char *aboutText1;
  char *aboutText2;
};
*/

#define YAM_URL "http://yam.ch/"

/* Private Functions */
/// UrlObject
// create a clickable object to go to YAM's homepage
static Object *UrlObject(void)
{
  Object *obj;

  ENTER();

  // try Urltext.mcc first
  if((obj = UrltextObject,
    MUIA_Urltext_Url, YAM_URL,
    End) == NULL)
  {
    // fall back to a simple text object
    obj = TextObject,
      MUIA_Text_PreParse, "\033c\033u\0335",
      MUIA_Text_Contents, YAM_URL,
      MUIA_Text_Copy, FALSE,
      MUIA_InputMode, MUIV_InputMode_RelVerify,
    End;
  }

  RETURN(obj);
  return obj;
}

///

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  char logopath[SIZE_PATHFILE];
  char *compileInfo;
  Object *bt_okay;
  Object *bt_gopage;
  Object *infoObject = NULL;

  // Now we create the about text
  //
  // Please note that the following text should *NOT*
  // be translated and is therefore not splitted into
  // locale strings in the YAM.cd file. People should not
  // mess around with the license and other stuff and therefore
  // we keep that text unlocalized.
  char *aboutText1;
  char *aboutText2;
  int aboutResult1;
  int aboutResult2;
  const char aboutTemplate1[] =
                          "\033b%s\033n\n" // current developers
                          "\n"
                          "Jens Langner\n"
                          "Thore Boeckelmann";
  const char aboutTemplate2[] =
                          "\n"
                          "\033b%s\033n\n" // contributors (alphabetic order - last name)
                          "\n"
                          "Olaf Barthel\n"
                          "Marcel Beck\n"
                          "Andrew Bell\n"
                          "Giles Burdett\n"
                          "Christian Hattemer\n"
                          "Alexey Ivanov\n"
                          "Jacob Laursen\n"
                          "Gunther Nikl\n"
                          "Allan Odgaard\n"
                          "David Rey\n"
                          "Matthias Rustler\n"
                          "Dirk Stoecker\n"
                          "Joerg Strohmayer\n"
                          "Frank Weber\n"
                          "\n"
                          "\033b%s\033n\n" // active translators (alphabetic order - language)
                          "\n"
                          "Vit Sindlar (czech)\n"
                          "Thore Boeckelmann (english-british)\n"
                          "Alexandre Balaban (french)\n"
                          "Thore Boeckelmann, Jens Langner (german)\n"
                          "Miklos Gubucz (hungarian)\n"
                          "Samir Hawamdeh (italian)\n"
                          "Emilio Jimenez (spanish)\n"
                          "Paer Boberg (swedish)\n"
                          "Alper Soenmez (turkish)\n"
                          "\n"
                          "%s\n" // GPL
                          "\n"
                          "%s\n" // 3rd party software
                          "\n"
                          "\033bMagic User Interface\033n\n"
                          "\033iStefan Stuntz, et al.\033n\n"
                          "\n"
                          "\033bTextEditor.mcc\033n\n"
                          "\033iTextEditor.mcc Open Source Team\033n\n"
                          "http://sf.net/p/texteditor-mcc/\n"
                          "\n"
                          "\033bBetterString.mcc\033n\n"
                          "\033iBetterString.mcc Open Source Team\033n\n"
                          "http://sf.net/p/bstring-mcc/\n"
                          "\n"
                          "\033bTheBar.mcc\033n\n"
                          "\033iTheBar.mcc Open Source Team\033n\n"
                          "http://sf.net/p/thebar/\n"
                          "\n"
                          "\033bNList MCC classes\033n\n"
                          "\033iNList Open Source Team\033n\n"
                          "http://sf.net/p/nlist-classes/\n"
                          "\n"
                          "\033bUrltext.mcc\033n\n"
                          "\033iAlfonso Ranieri\033n\n"
                          "http://digilander.libero.it/asoft/\n"
                          "\n"
                          "\033bcodesets.library\033n\n"
                          "\033icodesets.library Open Source Team\033n\n"
                          "http://sf.net/p/codesetslib/\n"
                          "\n"
                          "\033bxpkmaster.library\033n\n"
                          "\033iDirk Stoecker\033n\n"
                          "http://www.dstoecker.de/xpkmaster.html\n"
                          "\n"
                          "\033bamissl.library\033n\n"
                          "\033iAmiSSL Open Source Team\033n\n"
                          "http://sf.net/p/amissl/\n"
                          "\n"
                          "\033bopenurl.library\033n\n"
                          "\033iOpenURL Open Source Team\033n\n"
                          "http://sf.net/p/openurllib/\n"
                          "\n"
                          "\033bFlexCat\033n\n"
                          "\033iFlexCat Open Source Team\033n\n"
                          "http://sf.net/p/flexcat/\n"
                          "\n"
                          "\033bflex: The Fast Lexical Analyzer\033n\n"
                          "\033iflex Open Source Team\033n\n"
                          "http://flex.sourceforge.net/\n"
                          "\n"
                          "\033bexpat XML Parser library\033n\n"
                          "\033iexpat Open Source Team\033n\n"
                          "http://expat.sourceforge.net/\n"
                          "\n"
                          "\033bTime Zone Database\033n\n"
                          "\033iInternet Assigned Numbers Authority (IANA)\033n\n"
                          "http://www.iana.org/time-zones\n"
                          "\n"
                          "\033bPretty Good Privacy (PGP)\033n\n"
                          "\033iPhil Zimmermann\033n\n"
                          "\n"
                          "%s\n" // YAM news
                          "\n"
                          "\n"
                          "\n";

  ENTER();

  compileInfo = (char *)xget(G->App, MUIA_YAMApplication_CompileInfo);

  AddPath(logopath, G->ThemesDir, "default/logo", sizeof(logopath));

  // use asprintf() function to allocate&set the content of our
  // about text.
  aboutResult1 = asprintf(&aboutText1, aboutTemplate1,
    tr(MSG_ABOUT_CURRENT_DEVELOPERS));

  aboutResult2 = asprintf(&aboutText2, aboutTemplate2,
    tr(MSG_ABOUT_CONTRIBUTORS),
    tr(MSG_ABOUT_LOCALIZATION_CONTRIBUTORS),
    tr(MSG_ABOUT_GPL),
    tr(MSG_ABOUT_3RD_PARTY_SOFTWARE),
    tr(MSG_ABOUT_YAM_NEWS));

  if(aboutResult1 != -1 && aboutResult2 != -1)
  {
    // now we go and try to setup a crawling.mcc object
    // with the object text. However, if that fails we simply generate
    // and NFloattext object instead.

    infoObject = CrawlingObject,

      MUIA_Font,          MUIV_Font_Tiny,
      MUIA_FixHeightTxt,  aboutText1,

      Child, TextObject,
        MUIA_Font,          MUIV_Font_Tiny,
        MUIA_Text_PreParse, "\033c",
        MUIA_Text_Contents, aboutText1,
        MUIA_Text_SetMax,   FALSE,
        MUIA_Text_Copy,     FALSE,
      End,

      Child, TextObject,
        MUIA_Font,          MUIV_Font_Tiny,
        MUIA_Text_PreParse, "\033c",
        MUIA_Text_Contents, aboutText2,
        MUIA_Text_SetMax,   FALSE,
        MUIA_Text_Copy,     FALSE,
      End,

      Child, TextObject,
        MUIA_Font,          MUIV_Font_Tiny,
        MUIA_Text_PreParse, "\033c",
        MUIA_Text_Contents, aboutText1,
        MUIA_Text_SetMax,   FALSE,
        MUIA_Text_Copy,     FALSE,
      End,

    End;

    // if we weren't able to create a crawling object
    // we go and create a NFloattextObject instead
    if(infoObject == NULL)
    {
      char *aboutText;

      // combine the separated texts into one text
      if(asprintf(&aboutText, "%s%s", aboutText1, aboutText2) != -1)
      {
        infoObject = NListviewObject,

          MUIA_NListview_Horiz_ScrollBar, MUIV_NListview_HSB_Off,
          MUIA_NListview_NList, NFloattextObject,
            MUIA_Font,            MUIV_Font_Tiny,
            MUIA_NList_Format,    "P=\033c",
            MUIA_NList_Input,     FALSE,
            MUIA_NFloattext_Text, aboutText1,
          End,

        End;

        free(aboutText1);
        free(aboutText2);
        aboutText1 = aboutText;
        aboutText2 = NULL;
      }
    }
  }
  else
  {
    // make sure the pointer is NULL as asprintf() does not guarantee this
    if(aboutResult1 != -1)
      free(aboutText1);
    if(aboutResult2 != -1)
      free(aboutText2);

    aboutText1 = NULL;
    aboutText2 = NULL;
  }

  // create the main window object
  if((obj = DoSuperNew(cl, obj,

    MUIA_Window_Title,        tr(MSG_ABOUTYAM),
    MUIA_Window_CloseGadget,  FALSE,
    MUIA_Window_SizeGadget,   FALSE,
    MUIA_Window_LeftEdge,     MUIV_Window_LeftEdge_Centered,
    MUIA_Window_TopEdge,      MUIV_Window_TopEdge_Centered,
    MUIA_Window_Width,        MUIV_Window_Width_MinMax(0),

    WindowContents, VGroup,
      MUIA_Background, MUII_GroupBack,
      Child, HGroup,
        MUIA_Group_Spacing, 0,
        Child, HSpace(0),
        Child, MakeImageObject("logo", logopath),
        Child, HSpace(0),
      End,
      Child, HCenter((VGroup,
        Child, CLabel(tr(MSG_YAMINFO)),
        Child, CLabel(yamfullcopyright),
        Child, ColGroup(2),
          Child, bt_gopage = UrlObject(),
        End,
        Child, RectangleObject,
          MUIA_Rectangle_HBar, TRUE,
          MUIA_FixHeight, 8,
        End,
        Child, ColGroup(2),
          MUIA_Font, MUIV_Font_Tiny,
          MUIA_Group_HorizSpacing,  8,
          MUIA_Group_VertSpacing,   2,
          Child, Label(tr(MSG_Version)),
          Child, LLabel(yamversionver),
          Child, Label(tr(MSG_CompilationDate)),
          Child, LLabel(compileInfo),
        End,
      End)),

      Child, RectangleObject,
        MUIA_Rectangle_HBar, TRUE,
        MUIA_FixHeight, 8,
      End,

      Child, infoObject,

      Child, RectangleObject,
        MUIA_Rectangle_HBar, TRUE,
        MUIA_FixHeight, 8,
      End,

      Child, HGroup,
        Child, RectangleObject, End,
        Child, bt_okay = MakeButton(tr(MSG_Okay)),
        Child, RectangleObject, End,
      End,
    End,

    TAG_MORE, inittags(msg))) != NULL)
  {
    GETDATA;

    DoMethod(G->App, OM_ADDMEMBER, obj);

    data->aboutText1 = aboutText1;
    data->aboutText2 = aboutText2;

    DoMethod(obj,       MUIM_Notify, MUIA_Window_CloseRequest, TRUE, MUIV_Notify_Self, 3, MUIM_Set, MUIA_Window_Open, FALSE);
    DoMethod(bt_okay,   MUIM_Notify, MUIA_Pressed, FALSE, obj, 3, MUIM_Set, MUIA_Window_Open, FALSE);
    DoMethod(bt_gopage, MUIM_Notify, MUIA_Pressed, FALSE, obj, 1, MUIM_Aboutwindow_GotoSupportPage);

    xset(obj, MUIA_Window_Activate,      TRUE,
              MUIA_Window_DefaultObject, bt_okay);
  }

  RETURN((IPTR)obj);
  return (IPTR)obj;
}

///
/// OVERLOAD(OM_DISPOSE)
OVERLOAD(OM_DISPOSE)
{
  GETDATA;
  IPTR result;

  ENTER();

  free(data->aboutText1);
  free(data->aboutText2);

  result = DoSuperMethodA(cl, obj, msg);

  RETURN(result);
  return result;
}

///

/* Public Methods */
/// DECLARE(GotoSupportPage)
// open a browser and go to the YAM support page
// used if Urltext.mcc is not available
DECLARE(GotoSupportPage)
{
  ENTER();

  GotoURL(YAM_URL, FALSE);

  RETURN(0);
  return 0;
}

///
