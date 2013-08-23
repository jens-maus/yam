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
 YAM OpenSource project     : http://sourceforge.net/projects/yamos/

 $Id$

 Superclass:  MUIC_ConfigPage
 Description: "Read" configuration page

***************************************************************************/

#include "ReadConfigPage_cl.h"

#include <proto/muimaster.h>

#include "YAM.h"
#include "YAM_config.h"

#include "mui/ConfigPage.h"
#include "mui/ConfigPageList.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *CY_HEADER;
  Object *ST_HEADERS;
  Object *CY_SENDERINFO;
  Object *CH_WRAPHEAD;
  Object *CY_SIGSEPLINE;
  Object *CA_COLSIG;
  Object *CA_COLTEXT;
  Object *CA_COL1QUOT;
  Object *CA_COL2QUOT;
  Object *CA_COL3QUOT;
  Object *CA_COL4QUOT;
  Object *CA_COLURL;
  Object *CH_FIXFEDIT;
  Object *CH_TEXTCOLORS_READ;
  Object *CH_TEXTSTYLES_READ;
  Object *CH_MDN_NEVER;
  Object *CH_MDN_ALLOW;
  Object *CY_MDN_NORECIPIENT;
  Object *CY_MDN_NODOMAIN;
  Object *CY_MDN_DELETE;
  Object *CY_MDN_OTHER;
  Object *CH_MULTIWIN;
  Object *CH_GLOBALMAILTHREADS;
  Object *CH_DELAYEDSTATUS;
  Object *NB_DELAYEDSTATUS;
  Object *CH_CONVERTHTML;
  Object *CH_MAPFOREIGNCHARS;
  Object *CH_DETECTCYRILLIC;
  Object *CH_ALLTEXTS;
  Object *CH_SHOWALTPARTS;
};
*/

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  static const char *headopt[4];
  static const char *siopt[5];
  static const char *slopt[5];
  static const char *rropt[5];
  Object *CY_HEADER;
  Object *ST_HEADERS;
  Object *CY_SENDERINFO;
  Object *CH_WRAPHEAD;
  Object *CY_SIGSEPLINE;
  Object *CA_COLSIG;
  Object *CA_COLTEXT;
  Object *CA_COL1QUOT;
  Object *CA_COL2QUOT;
  Object *CA_COL3QUOT;
  Object *CA_COL4QUOT;
  Object *CA_COLURL;
  Object *CH_FIXFEDIT;
  Object *CH_TEXTCOLORS_READ;
  Object *CH_TEXTSTYLES_READ;
  Object *CH_MDN_NEVER;
  Object *CH_MDN_ALLOW;
  Object *CY_MDN_NORECIPIENT;
  Object *CY_MDN_NODOMAIN;
  Object *CY_MDN_DELETE;
  Object *CY_MDN_OTHER;
  Object *CH_MULTIWIN;
  Object *CH_GLOBALMAILTHREADS;
  Object *CH_DELAYEDSTATUS;
  Object *NB_DELAYEDSTATUS;
  Object *CH_CONVERTHTML;
  Object *CH_MAPFOREIGNCHARS;
  Object *CH_DETECTCYRILLIC;
  Object *CH_ALLTEXTS;
  Object *CH_SHOWALTPARTS;

  ENTER();

  headopt[0] = tr(MSG_CO_HeadNone);
  headopt[1] = tr(MSG_CO_HeadShort);
  headopt[2] = tr(MSG_CO_HeadFull);
  headopt[3] = NULL;

  siopt[0] = tr(MSG_CO_SINone);
  siopt[1] = tr(MSG_CO_SIFields);
  siopt[2] = tr(MSG_CO_SIAll);
  siopt[3] = tr(MSG_CO_SImageOnly);
  siopt[4] = NULL;

  slopt[SST_BLANK]= tr(MSG_CO_SLBlank);
  slopt[SST_DASH] = tr(MSG_CO_SLDash);
  slopt[SST_BAR]  = tr(MSG_CO_SLBar);
  slopt[SST_SKIP] = tr(MSG_CO_SLSkip);
  slopt[4] = NULL;

  rropt[0] = tr(MSG_CO_MDN_ACTION_IGNORE);
  rropt[1] = tr(MSG_CO_MDN_ACTION_SEND);
  rropt[2] = tr(MSG_CO_MDN_ACTION_QUEUE);
  rropt[3] = tr(MSG_CO_MDN_ACTION_ASK);
  rropt[4] = NULL;

  if((obj = DoSuperNew(cl, obj,
    MUIA_HelpNode, "Configuration#Read",
    MUIA_ConfigPage_Page, cp_Read,
    MUIA_ConfigPage_Contents, VGroup,
      Child, ColGroup(3), GroupFrameT(tr(MSG_CO_HeaderLayout)),
        Child, Label2(tr(MSG_CO_Header)),
        Child, CY_HEADER = MakeCycle(headopt,tr(MSG_CO_Header)),
        Child, ST_HEADERS = MakeString(SIZE_PATTERN, ""),
        Child, Label1(tr(MSG_CO_SenderInfo)),
        Child, CY_SENDERINFO = MakeCycle(siopt,tr(MSG_CO_SenderInfo)),
        Child, MakeCheckGroup(&CH_WRAPHEAD, tr(MSG_CO_WrapHeader)),
      End,

      Child, VGroup, GroupFrameT(tr(MSG_CO_BodyLayout)),
        Child, ColGroup(2),
          Child, Label1(tr(MSG_CO_SignatureSep)),
          Child, HGroup,
            Child, CY_SIGSEPLINE = MakeCycle(slopt,tr(MSG_CO_SignatureSep)),
            Child, CA_COLSIG = PoppenObject,
              MUIA_CycleChain, TRUE,
            End,
          End,

          Child, Label1(tr(MSG_CO_ColoredText)),
          Child, CA_COLTEXT = PoppenObject,
            MUIA_CycleChain, TRUE,
          End,

          Child, Label1(tr(MSG_CO_OldQuotes)),
          Child, HGroup,
            Child, CA_COL1QUOT = PoppenObject,
              MUIA_CycleChain, TRUE,
            End,
            Child, CA_COL2QUOT = PoppenObject,
              MUIA_CycleChain, TRUE,
            End,
            Child, CA_COL3QUOT = PoppenObject,
              MUIA_CycleChain, TRUE,
            End,
            Child, CA_COL4QUOT = PoppenObject,
              MUIA_CycleChain, TRUE,
            End,
          End,

          Child, Label1(tr(MSG_CO_URLCOLOR)),
          Child, CA_COLURL = PoppenObject,
            MUIA_CycleChain, TRUE,
          End,

       End,

        Child, RectangleObject,
          MUIA_VertWeight,         0,
          MUIA_Rectangle_HBar,     TRUE,
          MUIA_Rectangle_BarTitle, tr(MSG_CO_FONTSETTINGS),
        End,
        Child, MakeCheckGroup(&CH_FIXFEDIT, tr(MSG_CO_FixedFontEdit)),
        Child, MakeCheckGroup(&CH_TEXTCOLORS_READ, tr(MSG_CO_TEXTCOLORS_READ)),
        Child, MakeCheckGroup(&CH_TEXTSTYLES_READ, tr(MSG_CO_TEXTSTYLES_READ)),

      End,

      Child, VGroup, GroupFrameT(tr(MSG_CO_MDN_TITLE)),
        Child, ColGroup(2),
          Child, LLabel(tr(MSG_CO_MDN_DESCRIPTION)),
          Child, HSpace(0),

          Child, ColGroup(4),
            Child, HSpace(1),
            Child, CH_MDN_NEVER = MakeCheck(tr(MSG_CO_MDN_DISABLED)),
            Child, LLabel(tr(MSG_CO_MDN_DISABLED)),
            Child, HSpace(0),

            Child, HSpace(1),
            Child, CH_MDN_ALLOW = MakeCheck(tr(MSG_CO_MDN_ENABLED)),
            Child, LLabel(tr(MSG_CO_MDN_ENABLED)),
            Child, HSpace(0),

            Child, HSpace(1),
            Child, HSpace(0),
            Child, LLabel(tr(MSG_CO_MDN_NORECIPIENT)),
            Child, CY_MDN_NORECIPIENT = MakeCycle(rropt, tr(MSG_CO_MDN_NORECIPIENT)),

            Child, HSpace(1),
            Child, HSpace(0),
            Child, LLabel(tr(MSG_CO_MDN_NODOMAIN)),
            Child, CY_MDN_NODOMAIN = MakeCycle(rropt, tr(MSG_CO_MDN_NODOMAIN)),

            Child, HSpace(1),
            Child, HSpace(0),
            Child, LLabel(tr(MSG_CO_MDN_DELETE)),
            Child, CY_MDN_DELETE = MakeCycle(rropt, tr(MSG_CO_MDN_DELETE)),

            Child, HSpace(1),
            Child, HSpace(0),
            Child, LLabel(tr(MSG_CO_MDN_OTHER)),
            Child, CY_MDN_OTHER = MakeCycle(rropt, tr(MSG_CO_MDN_OTHER)),

          End,
          Child, HSpace(0),

        End,
      End,

      Child, VGroup, GroupFrameT(tr(MSG_CO_OtherOptions)),
        Child, MakeCheckGroup(&CH_MULTIWIN, tr(MSG_CO_MultiReadWin)),
        Child, MakeCheckGroup(&CH_GLOBALMAILTHREADS, tr(MSG_CO_GLOBALMAILTHREADS)),
        Child, HGroup,
          Child, CH_DELAYEDSTATUS = MakeCheck(tr(MSG_CO_SETSTATUSDELAYED1)),
          Child, Label2(tr(MSG_CO_SETSTATUSDELAYED1)),
          Child, NB_DELAYEDSTATUS = NumericbuttonObject,
            MUIA_CycleChain,  TRUE,
            MUIA_Numeric_Min, 1,
            MUIA_Numeric_Max, 10,
          End,
          Child, Label2(tr(MSG_CO_SETSTATUSDELAYED2)),
          Child, HSpace(0),
        End,
        Child, MakeCheckGroup(&CH_CONVERTHTML, tr(MSG_CO_CONVERTHTML)),
        Child, MakeCheckGroup(&CH_MAPFOREIGNCHARS, tr(MSG_CO_MAPFOREIGNCHARS)),
        Child, MakeCheckGroup(&CH_DETECTCYRILLIC, tr(MSG_CO_DETECT_CYRILLIC)),
        Child, MakeCheckGroup(&CH_ALLTEXTS, tr(MSG_CO_DisplayAll)),
        Child, MakeCheckGroup(&CH_SHOWALTPARTS, tr(MSG_CO_SHOWALTPARTS)),
      End,
    End,
  TAG_MORE, inittags(msg))) != NULL)
  {
    GETDATA;

    data->CY_HEADER =            CY_HEADER;
    data->ST_HEADERS =           ST_HEADERS;
    data->CY_SENDERINFO =        CY_SENDERINFO;
    data->CH_WRAPHEAD =          CH_WRAPHEAD;
    data->CY_SIGSEPLINE =        CY_SIGSEPLINE;
    data->CA_COLSIG =            CA_COLSIG;
    data->CA_COLTEXT =           CA_COLTEXT;
    data->CA_COL1QUOT =          CA_COL1QUOT;
    data->CA_COL2QUOT =          CA_COL2QUOT;
    data->CA_COL3QUOT =          CA_COL3QUOT;
    data->CA_COL4QUOT =          CA_COL4QUOT;
    data->CA_COLURL =            CA_COLURL;
    data->CH_FIXFEDIT =          CH_FIXFEDIT;
    data->CH_TEXTCOLORS_READ =   CH_TEXTCOLORS_READ;
    data->CH_TEXTSTYLES_READ =   CH_TEXTSTYLES_READ;
    data->CH_MDN_NEVER =         CH_MDN_NEVER;
    data->CH_MDN_ALLOW =         CH_MDN_ALLOW;
    data->CY_MDN_NORECIPIENT =   CY_MDN_NORECIPIENT;
    data->CY_MDN_NODOMAIN =      CY_MDN_NODOMAIN;
    data->CY_MDN_DELETE =        CY_MDN_DELETE;
    data->CY_MDN_OTHER =         CY_MDN_OTHER;
    data->CH_MULTIWIN =          CH_MULTIWIN;
    data->CH_GLOBALMAILTHREADS = CH_GLOBALMAILTHREADS;
    data->CH_DELAYEDSTATUS =     CH_DELAYEDSTATUS;
    data->NB_DELAYEDSTATUS =     NB_DELAYEDSTATUS;
    data->CH_CONVERTHTML =       CH_CONVERTHTML;
    data->CH_MAPFOREIGNCHARS =   CH_MAPFOREIGNCHARS;
    data->CH_DETECTCYRILLIC =    CH_DETECTCYRILLIC;
    data->CH_ALLTEXTS =          CH_ALLTEXTS;
    data->CH_SHOWALTPARTS =      CH_SHOWALTPARTS;

    set(ST_HEADERS, MUIA_Disabled, TRUE);

    SetHelp(CY_HEADER,          MSG_HELP_CO_CY_HEADER);
    SetHelp(ST_HEADERS,         MSG_HELP_CO_ST_HEADERS);
    SetHelp(CY_SENDERINFO,      MSG_HELP_CO_CY_SENDERINFO);
    SetHelp(CA_COLSIG,          MSG_HELP_CO_CA_COLSIG);
    SetHelp(CA_COLTEXT,         MSG_HELP_CO_CA_COLTEXT);
    SetHelp(CA_COL1QUOT,        MSG_HELP_CO_CA_COL1QUOT);
    SetHelp(CA_COL2QUOT,        MSG_HELP_CO_CA_COL2QUOT);
    SetHelp(CA_COL3QUOT,        MSG_HELP_CO_CA_COL3QUOT);
    SetHelp(CA_COL4QUOT,        MSG_HELP_CO_CA_COL4QUOT);
    SetHelp(CA_COLURL,          MSG_HELP_CO_CA_COLURL);
    SetHelp(CH_ALLTEXTS,        MSG_HELP_CO_CH_ALLTEXTS);
    SetHelp(CH_MULTIWIN,        MSG_HELP_CO_CH_MULTIWIN);
    SetHelp(CY_SIGSEPLINE,      MSG_HELP_CO_CY_SIGSEPLINE);
    SetHelp(CH_FIXFEDIT,        MSG_HELP_CO_CH_FIXFEDIT);
    SetHelp(CH_WRAPHEAD,        MSG_HELP_CO_CH_WRAPHEAD);
    SetHelp(CH_TEXTSTYLES_READ, MSG_HELP_CO_CH_TEXTSTYLES_READ);
    SetHelp(CH_TEXTCOLORS_READ, MSG_HELP_CO_CH_TEXTCOLORS_READ);
    SetHelp(CH_SHOWALTPARTS,    MSG_HELP_CO_CH_SHOWALTPARTS);
    SetHelp(CH_DELAYEDSTATUS,   MSG_HELP_CO_SETSTATUSDELAYED);
    SetHelp(NB_DELAYEDSTATUS,   MSG_HELP_CO_SETSTATUSDELAYED);
    SetHelp(CH_CONVERTHTML,     MSG_HELP_CO_CONVERTHTML);
    SetHelp(CH_MDN_NEVER,       MSG_HELP_CO_CH_MDN_NEVER);
    SetHelp(CH_MDN_ALLOW,       MSG_HELP_CO_CH_MDN_ALLOW);
    SetHelp(CY_MDN_NORECIPIENT, MSG_HELP_CO_CY_MDN_NORECIPIENT);
    SetHelp(CY_MDN_NODOMAIN,    MSG_HELP_CO_CY_MDN_NODOMAIN);
    SetHelp(CY_MDN_DELETE,      MSG_HELP_CO_CY_MDN_DELETE);
    SetHelp(CY_MDN_OTHER,       MSG_HELP_CO_CY_MDN_OTHER);
    SetHelp(CH_MAPFOREIGNCHARS, MSG_HELP_CO_MAPFOREIGNCHARS);
    SetHelp(CH_DETECTCYRILLIC,  MSG_HELP_CO_DETECT_CYRILLIC);
    SetHelp(CH_GLOBALMAILTHREADS, MSG_HELP_CO_CH_GLOBALMAILTHREADS);

    // disable all poppen objects in case the textstyles checkbox is disabled
    DoMethod(CH_TEXTCOLORS_READ, MUIM_Notify, MUIA_Selected, MUIV_EveryTime,
             obj, 11, MUIM_MultiSet, MUIA_Disabled, MUIV_NotTriggerValue, CA_COLSIG,
                                                                          CA_COLTEXT,
                                                                          CA_COL1QUOT,
                                                                          CA_COL2QUOT,
                                                                          CA_COL3QUOT,
                                                                          CA_COL4QUOT,
                                                                          CA_COLURL,
                                                                          NULL);

    DoMethod(CY_HEADER, MUIM_Notify, MUIA_Cycle_Active, 0, obj, 7, MUIM_MultiSet, MUIA_Disabled, TRUE, ST_HEADERS,
                                                                                                       CH_WRAPHEAD,
                                                                                                       CY_SENDERINFO,
                                                                                                       NULL);
    DoMethod(CY_HEADER, MUIM_Notify, MUIA_Cycle_Active, 1, obj, 7, MUIM_MultiSet, MUIA_Disabled, FALSE, ST_HEADERS,
                                                                                                        CH_WRAPHEAD,
                                                                                                        CY_SENDERINFO,
                                                                                                        NULL);
    DoMethod(CY_HEADER, MUIM_Notify, MUIA_Cycle_Active, 2, obj, 5, MUIM_MultiSet, MUIA_Disabled, TRUE, ST_HEADERS,
                                                                                                       NULL);
    DoMethod(CY_HEADER, MUIM_Notify, MUIA_Cycle_Active, 2, obj, 6, MUIM_MultiSet, MUIA_Disabled, FALSE, CH_WRAPHEAD,
                                                                                                        CY_SENDERINFO,
                                                                                                        NULL);

    // setup the MDN stuff
    DoMethod(CH_MDN_NEVER, MUIM_Notify, MUIA_Selected, MUIV_EveryTime, obj, 2, METHOD(UpdateMDN), MUIV_NotTriggerValue);
    DoMethod(CH_MDN_ALLOW, MUIM_Notify, MUIA_Selected, MUIV_EveryTime, obj, 2, METHOD(UpdateMDN), MUIV_NotTriggerValue);

    DoMethod(CH_DELAYEDSTATUS, MUIM_Notify, MUIA_Selected, MUIV_EveryTime, NB_DELAYEDSTATUS, 3, MUIM_Set, MUIA_Disabled, MUIV_NotTriggerValue);
  }

  RETURN((IPTR)obj);
  return (IPTR)obj;
}

///
/// OVERLOAD(MUIM_ConfigPage_ConfigToGUI)
OVERLOAD(MUIM_ConfigPage_ConfigToGUI)
{
  GETDATA;

  ENTER();

  setcycle(data->CY_HEADER, CE->ShowHeader);
  setstring(data->ST_HEADERS, CE->ShortHeaders);
  setcycle(data->CY_SENDERINFO, CE->ShowSenderInfo);
  setcycle(data->CY_SIGSEPLINE, CE->SigSepLine);
  set(data->CA_COLSIG,   MUIA_Pendisplay_Spec, &CE->ColorSignature);
  set(data->CA_COLTEXT,  MUIA_Pendisplay_Spec, &CE->ColoredText);
  set(data->CA_COL1QUOT, MUIA_Pendisplay_Spec, &CE->Color1stLevel);
  set(data->CA_COL2QUOT, MUIA_Pendisplay_Spec, &CE->Color2ndLevel);
  set(data->CA_COL3QUOT, MUIA_Pendisplay_Spec, &CE->Color3rdLevel);
  set(data->CA_COL4QUOT, MUIA_Pendisplay_Spec, &CE->Color4thLevel);
  set(data->CA_COLURL,   MUIA_Pendisplay_Spec, &CE->ColorURL);
  setcheckmark(data->CH_ALLTEXTS, CE->DisplayAllTexts);
  setcheckmark(data->CH_FIXFEDIT, CE->FixedFontEdit);
  setcheckmark(data->CH_WRAPHEAD, CE->WrapHeader);
  setcheckmark(data->CH_TEXTSTYLES_READ, CE->UseTextStylesRead);
  setcheckmark(data->CH_TEXTCOLORS_READ, CE->UseTextColorsRead);

  // disable all poppen objects according to the UseTextColorsRead setting
  DoMethod(obj, MUIM_MultiSet, MUIA_Disabled, CE->UseTextColorsRead == FALSE, data->CA_COLSIG,
                                                                              data->CA_COLTEXT,
                                                                              data->CA_COL1QUOT,
                                                                              data->CA_COL2QUOT,
                                                                              data->CA_COL3QUOT,
                                                                              data->CA_COL4QUOT,
                                                                              data->CA_COLURL,
                                                                              NULL);

  setcheckmark(data->CH_SHOWALTPARTS, CE->DisplayAllAltPart);

  // set the MDN stuff according to other config
  setcheckmark(data->CH_MDN_NEVER, CE->MDNEnabled == FALSE);
  setcheckmark(data->CH_MDN_ALLOW, CE->MDNEnabled == TRUE);
  setcycle(data->CY_MDN_NORECIPIENT, CE->MDN_NoRecipient);
  setcycle(data->CY_MDN_NODOMAIN, CE->MDN_NoDomain);
  setcycle(data->CY_MDN_DELETE, CE->MDN_OnDelete);
  setcycle(data->CY_MDN_OTHER, CE->MDN_Other);

  setcheckmark(data->CH_MULTIWIN, CE->MultipleReadWindows);
  setcheckmark(data->CH_DELAYEDSTATUS, CE->StatusChangeDelayOn);

  xset(data->NB_DELAYEDSTATUS, MUIA_Numeric_Value, CE->StatusChangeDelay / 1000,
							   MUIA_Disabled, CE->StatusChangeDelayOn == FALSE);

  setcheckmark(data->CH_CONVERTHTML, CE->ConvertHTML);

  set(data->ST_HEADERS, MUIA_Disabled, CE->ShowHeader == HM_NOHEADER || CE->ShowHeader == HM_FULLHEADER);
  set(data->CY_SENDERINFO, MUIA_Disabled, CE->ShowHeader == HM_NOHEADER);
  set(data->CH_WRAPHEAD, MUIA_Disabled, CE->ShowHeader == HM_NOHEADER);
  setcheckmark(data->CH_DETECTCYRILLIC, CE->DetectCyrillic);
  setcheckmark(data->CH_MAPFOREIGNCHARS, CE->MapForeignChars);
  setcheckmark(data->CH_GLOBALMAILTHREADS, CE->GlobalMailThreads);

  RETURN(0);
  return 0;
}

///
/// OVERLOAD(MUIM_ConfigPage_GUIToConfig)
OVERLOAD(MUIM_ConfigPage_GUIToConfig)
{
  GETDATA;

  ENTER();

  CE->ShowHeader        = GetMUICycle(data->CY_HEADER);
  GetMUIString(CE->ShortHeaders, data->ST_HEADERS, sizeof(CE->ShortHeaders));
  CE->ShowSenderInfo    = GetMUICycle(data->CY_SENDERINFO);
  CE->SigSepLine        = GetMUICycle(data->CY_SIGSEPLINE);
  memcpy(&CE->ColorSignature, (struct MUI_PenSpec*)xget(data->CA_COLSIG,   MUIA_Pendisplay_Spec), sizeof(struct MUI_PenSpec));
  memcpy(&CE->ColoredText,    (struct MUI_PenSpec*)xget(data->CA_COLTEXT,  MUIA_Pendisplay_Spec), sizeof(struct MUI_PenSpec));
  memcpy(&CE->Color1stLevel,  (struct MUI_PenSpec*)xget(data->CA_COL1QUOT, MUIA_Pendisplay_Spec), sizeof(struct MUI_PenSpec));
  memcpy(&CE->Color2ndLevel,  (struct MUI_PenSpec*)xget(data->CA_COL2QUOT, MUIA_Pendisplay_Spec), sizeof(struct MUI_PenSpec));
  memcpy(&CE->Color3rdLevel,  (struct MUI_PenSpec*)xget(data->CA_COL3QUOT, MUIA_Pendisplay_Spec), sizeof(struct MUI_PenSpec));
  memcpy(&CE->Color4thLevel,  (struct MUI_PenSpec*)xget(data->CA_COL4QUOT, MUIA_Pendisplay_Spec), sizeof(struct MUI_PenSpec));
  memcpy(&CE->ColorURL,       (struct MUI_PenSpec*)xget(data->CA_COLURL,   MUIA_Pendisplay_Spec), sizeof(struct MUI_PenSpec));
  CE->DisplayAllTexts   = GetMUICheck(data->CH_ALLTEXTS);
  CE->FixedFontEdit     = GetMUICheck(data->CH_FIXFEDIT);
  CE->WrapHeader        = GetMUICheck(data->CH_WRAPHEAD);
  CE->UseTextStylesRead = GetMUICheck(data->CH_TEXTSTYLES_READ);
  CE->UseTextColorsRead = GetMUICheck(data->CH_TEXTCOLORS_READ);
  CE->DisplayAllAltPart = GetMUICheck(data->CH_SHOWALTPARTS);

  // get MDN options from GUI
  CE->MDNEnabled      = GetMUICheck(data->CH_MDN_ALLOW) && !GetMUICheck(data->CH_MDN_NEVER);
  CE->MDN_NoRecipient = GetMUICycle(data->CY_MDN_NORECIPIENT);
  CE->MDN_NoDomain    = GetMUICycle(data->CY_MDN_NODOMAIN);
  CE->MDN_OnDelete    = GetMUICycle(data->CY_MDN_DELETE);
  CE->MDN_Other       = GetMUICycle(data->CY_MDN_OTHER);

  CE->MultipleReadWindows = GetMUICheck(data->CH_MULTIWIN);
  CE->StatusChangeDelayOn = GetMUICheck(data->CH_DELAYEDSTATUS);
  CE->StatusChangeDelay   = GetMUINumer(data->NB_DELAYEDSTATUS)*1000;
  CE->ConvertHTML         = GetMUICheck(data->CH_CONVERTHTML);

  CE->DetectCyrillic    = GetMUICheck(data->CH_DETECTCYRILLIC);
  CE->MapForeignChars   = GetMUICheck(data->CH_MAPFOREIGNCHARS);
  CE->GlobalMailThreads = GetMUICheck(data->CH_GLOBALMAILTHREADS);

  RETURN(0);
  return 0;
}

///
/// DECLARE(UpdateMDN)
// update the MDN cycle gadgets according to the "never send" and "allow" check marks
DECLARE(UpdateMDN) // ULONG active
{
  GETDATA;

  ENTER();

  nnset(data->CH_MDN_NEVER, MUIA_Selected, msg->active == FALSE);
  nnset(data->CH_MDN_ALLOW, MUIA_Selected, msg->active == TRUE);
  DoMethod(G->App, MUIM_MultiSet, MUIA_Disabled, msg->active == FALSE, data->CY_MDN_NORECIPIENT,
                                                                       data->CY_MDN_NODOMAIN,
                                                                       data->CY_MDN_DELETE,
                                                                       data->CY_MDN_OTHER,
                                                                       NULL);

  RETURN(0);
  return 0;
}

///
