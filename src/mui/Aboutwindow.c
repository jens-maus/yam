
/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2010 by YAM Open Source Team

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

#include <proto/muimaster.h>
#include <mui/NFloattext_mcc.h>

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
  char *aboutText;
};
*/

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  char logopath[SIZE_PATHFILE];
  char *compileInfo;
  Object *bt_okay;
  Object *bt_gopage;
  Object *infoObject;

  // Now we create the about text
  //
  // Please note that the following text should *NOT*
  // be translated and is therefore not splitted into
  // locale strings in the YAM.cd file. People should not
  // mess around with the license and other stuff and therefore
  // we keep that text unlocalized.
  char *aboutText;
  const char *aboutTemplate =
                          "\033b%s\033n\n\n"
                          "Jens Langner\n"
                          "Thore Boeckelmann\n"
                          "Matthias Rustler\n"
                          "Frank Weber\n"
                          "\n"
                          "\033b%s\033n\n\n"
                          "Gunther Nikl\n"
                          "Alexey Ivanov\n"
                          "David Rey\n"
                          "Dirk Stoecker\n"
                          "Christian Hattemer\n"
                          "Jacob Laursen\n"
                          "Joerg Strohmayer\n"
                          "Andrew Bell\n"
                          "Allan Odgaard\n"
                          "Giles Burdett\n"
                          "Olaf Barthel\n"
                          "\n"
                          "\033b%s\033n\n\n"
                          "Alexandre Balaban (french)\n"
                          "Par Boberg (swedish)\n"
                          "Luca Longone (italian)\n"
                          "Samir Hawamdeh (italian)\n"
                          "Antonis Iliakis (greek)\n"
                          "Vit Sindlar (czech)\n"
                          "Johan Banis (dutch)\n"
                          "Mariusz Danilewicz (polish)\n"
                          "Emilio Jimenez (spanish)\n"
                          "Alper Sonmez (turkish)\n"
                          "\n"
                          "%s\n"
                          "\n"
                          "%s\n"
                          "\n"
                          "\033bMagic User Interface\033n\n"
                          "\033iStefan Stuntz\033n\n"
                          "http://www.sasg.com/\n\n"
                          "\033bTextEditor.mcc\033n\n"
                          "\033iTextEditor.mcc Open Source Team\033n\n"
                          "http://www.sf.net/projects/texteditor-mcc/\n\n"
                          "\033bBetterString.mcc\033n\n"
                          "\033iBetterString.mcc Open Source Team\033n\n"
                          "http://www.sf.net/projects/bstring-mcc/\n\n"
                          "\033bTheBar.mcc\033n\n"
                          "\033iTheBar.mcc Open Source Team\033n\n"
                          "http://www.sf.net/projects/thebar/\n\n"
                          "\033bNList MCC classes\033n\n"
                          "\033iNList Open Source Team\033n\n"
                          "http://www.sf.net/projects/nlist-classes/\n\n"
                          "\033bcodesets.library\033n\n"
                          "\033icodesets.library Open Source Team\033n\n"
                          "http://www.sf.net/projects/codesetslib/\n\n"
                          "\033bxpkmaster.library\033n\n"
                          "\033iDirk Stoecker\033n\n"
                          "http://www.dstoecker.de/xpkmaster.html\n\n"
                          "\033bamissl.library\033n\n"
                          "\033iAndrija Antonijevic, Stefan Burstroem\033n\n"
                          "http://www.heightanxiety.com/AmiSSL/\n\n"
                          "\033bopenurl.library\033n\n"
                          "\033iOpenURL Open Source Team\033n\n"
                          "http://www.sf.net/projects/openurllib/\n\n"
                          "\033bSetDST\033n\n"
                          "\033iStefan Falke\033n\n"
                          "http://www.sfxsoft.de/setdst.html\n\n"
                          "\033bFlexCat\033n\n"
                          "\033iFlexCat Open Source Team\033n\n"
                          "http://www.sf.net/projects/flexcat/\n\n"
                          "\033bflex: The Fast Lexical Analyzer\033n\n"
                          "\033iflex Open Source Team\033n\n"
                          "http://flex.sourceforge.net/\n\n"
                          "\033bexpat XML Parser library\033n\n"
                          "\033iexpat Open Source Team\033n\n"
                          "http://expat.sourceforge.net/\n\n"
                          "\033bPretty Good Privacy (PGP)\033n\n"
                          "\033iPhil Zimmermann\033n\n\n"
                          "%s"
                          "\n\n\n\n\n\n\n\n\n\n";

  ENTER();

  compileInfo = (char *)xget(G->App, MUIA_YAMApplication_CompileInfo);

  AddPath(logopath, G->ProgDir, "Themes/default/logo", sizeof(logopath));

  // use asprintf() function to allocate&set the content of our
  // about text.
  asprintf(&aboutText, aboutTemplate, tr(MSG_ABOUT_CURRENT_DEVELOPERS),
                                      tr(MSG_ABOUT_CONTRIBUTORS),
                                      tr(MSG_ABOUT_LOCALIZATION_CONTRIBUTORS),
                                      tr(MSG_ABOUT_GPL),
                                      tr(MSG_ABOUT_3RD_PARTY_SOFTWARE),
                                      tr(MSG_ABOUT_YAM_NEWS));

  // now we go and try to setup a crawling.mcc object
  // with the object text. However, if that fails we simply generate
  // and NFloattext object instead.

  #ifndef MUIC_Crawling
  #define MUIC_Crawling "Crawling.mcc"
  #define CrawlingObject MUI_NewObject(MUIC_Crawling
  #endif

  infoObject = CrawlingObject,

    MUIA_Font,          MUIV_Font_Tiny,
    MUIA_FixHeightTxt,  "\n\n\n\n\n\n\n\n",
    MUIA_FixWidthTxt,   aboutText,

    Child, TextObject,
      MUIA_Font,          MUIV_Font_Tiny,
      MUIA_Text_PreParse, "\033c",
      MUIA_Text_Contents, aboutText,
      MUIA_Text_SetMax,   FALSE,
      MUIA_Text_Copy,     FALSE,
    End,

  End;

  // if we weren't able to create a crawling object
  // we go and create a NFloattextObject instead
  if(infoObject == NULL)
  {
    infoObject = NListviewObject,

      MUIA_NListview_Horiz_ScrollBar, MUIV_NListview_HSB_Off,
      MUIA_NListview_NList, NFloattextObject,
        MUIA_Font,            MUIV_Font_Tiny,
        MUIA_NList_Format,    "P=\033c",
        MUIA_NList_Input,     FALSE,
        MUIA_NFloattext_Text, aboutText,
      End,

    End;
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
          Child, bt_gopage = TextObject,
            MUIA_Text_Contents, "\033c\033u\0335http://www.yam.ch/\033n",
            MUIA_InputMode,     MUIV_InputMode_RelVerify,
          End,
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

    data->aboutText = aboutText;

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

  free(data->aboutText);

  result = DoSuperMethodA(cl, obj, msg);

  RETURN(result);
  return result;
}

///

/* Private Functions */

/* Public Methods */
/// DECLARE(GotoSupportPage)
// open a browser and go to the YAM support page
DECLARE(GotoSupportPage)
{
  ENTER();

  GotoURL("http://www.yam.ch/", FALSE);

  LEAVE();
  return 0;
}

///
