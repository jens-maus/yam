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
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 YAM Official Support Site :  http://www.yam.ch
 YAM OpenSource project    :  http://sourceforge.net/projects/yamos/

 $Id$

***************************************************************************/

#include "YAM.h"
#include "YAM_rexx.h"

/***************************************************************************
 Module: Root
***************************************************************************/

/// Global Vars
//  Defines global variables and structures
__near long __YAM_STACK = 32768;
__near long __buffsize = 8192;
__near long __MemPoolPuddleSize = 16384;

struct Library *WorkbenchBase = NULL, *IconBase = NULL;
struct Library *DataTypesBase = NULL, *MUIMasterBase = NULL, *XpkBase = NULL;
struct Library *OpenURLBase = NULL, *SocketBase = NULL, *CManagerBase = NULL;
struct LocaleBase *LocaleBase = NULL;
struct IntuitionBase *IntuitionBase = NULL;
struct PopupMenuBase *PopupMenuBase = NULL;

BOOL yamFirst = TRUE, yamLast = FALSE;

struct Global *G;
struct Config *C, *CE;

char *Status[9] = { "U","O","F","R","W","E","H","S","N" };
char *SigNames[3] = { ".signature", ".altsignature1", ".altsignature2" };
char *FolderNames[4] = { "incoming", "outgoing", "sent", "deleted" };

char *ContType[MAXCTYPE+1] =
{
   /*CT_TX_PLAIN */ "text/plain",
   /*CT_TX_HTML  */ "text/html",
   /*CT_TX_GUIDE */ "text/x-aguide",
   /*CT_AP_OCTET */ "application/octet-stream",
   /*CT_AP_PS    */ "application/postscript",
   /*CT_AP_RTF   */ "application/rtf",
   /*CT_AP_LHA   */ "application/x-lha",
   /*CT_AP_LZX   */ "application/x-lzx",
   /*CT_AP_ZIP   */ "application/x-zip",
   /*CT_AP_AEXE  */ "application/x-amiga-executable",
   /*CT_AP_SCRIPT*/ "application/x-amigados-script",
   /*CT_AP_REXX  */ "application/x-rexx",
   /*CT_IM_JPG   */ "image/jpeg",
   /*CT_IM_GIF   */ "image/gif",
   /*CT_IM_PNG   */ "image/png",
   /*CT_IM_TIFF  */ "image/tiff",
   /*CT_IM_ILBM  */ "image/x-ilbm",
   /*CT_AU_AU    */ "audio/basic",
   /*CT_AU_8SVX  */ "audio/x-8svx",
   /*CT_AU_WAV   */ "audio/x-wav",
   /*CT_VI_MPG   */ "video/mpeg",
   /*CT_VI_MOV   */ "video/quicktime",
   /*CT_VI_ANIM  */ "video/x-anim",
   /*CT_VI_AVI   */ "video/x-msvideo",
   /*CT_ME_EMAIL */ "message/rfc822",
   NULL,
};
APTR ContTypeDesc[MAXCTYPE] =
{
   MSG_CTtextplain, MSG_CTtexthtml, MSG_CTtextaguide,
   MSG_CTapplicationoctetstream, MSG_CTapplicationpostscript, MSG_CTapplicationrtf, MSG_CTapplicationlha, MSG_CTapplicationlzx, MSG_CTapplicationzip, MSG_CTapplicationamigaexe, MSG_CTapplicationadosscript, MSG_CTapplicationrexx,
   MSG_CTimagejpeg, MSG_CTimagegif, MSG_CTimagepng, MSG_CTimagetiff, MSG_CTimageilbm,
   MSG_CTaudiobasic, MSG_CTaudio8svx, MSG_CTaudiowav,
   MSG_CTvideompeg, MSG_CTvideoquicktime, MSG_CTvideoanim, MSG_CTvideomsvideo,
   MSG_CTmessagerfc822
};
char *wdays[7] = { "Sun","Mon","Tue","Wed","Thu","Fri","Sat" };
char *months[12] = { "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec" };
char *SecCodes[5] = { "none","sign","encrypt","sign+encrypt","anonymous" };
///
/// Timer Class
struct TC_Data
{
   struct MsgPort     *port;
   struct timerequest *req;
} TCData = { NULL,NULL };

//  Start a one second delay
void TC_Start(void)
{
   TCData.req->tr_node.io_Command = TR_ADDREQUEST;
   TCData.req->tr_time.tv_secs    = 1;
   TCData.req->tr_time.tv_micro   = 0;
   SendIO((struct IORequest *)TCData.req);
}

//  Frees timer resources
void TC_Exit(void)
{
   if (TCData.port)
   {
      if (TCData.req)
      {
         if (CheckIO((struct IORequest *)TCData.req)) return;
         AbortIO((struct IORequest *)TCData.req);
         WaitIO((struct IORequest *)TCData.req);
         CloseDevice((struct IORequest *)TCData.req);
         DeleteExtIO((struct IORequest *)TCData.req);
      }
      DeleteMsgPort(TCData.port);
   }
   TCData.port = NULL;
   TCData.req = NULL;
}

//  Initializes timer resources
BOOL TC_Init(void)
{
   if ((TCData.port = CreateMsgPort()))
      if ((TCData.req = (struct timerequest *)CreateExtIO(TCData.port, sizeof(struct timerequest))))
         if (!OpenDevice(TIMERNAME, UNIT_VBLANK, (struct IORequest *)TCData.req, 0))
            return TRUE;
   return FALSE;
}

//  Returns TRUE if the internal editor is currently being used
BOOL TC_ActiveEditor(int wrwin)
{
   if (G->WR[wrwin])
   {
      APTR ao;
      get(G->WR[wrwin]->GUI.WI, MUIA_Window_ActiveObject, &ao);
      return (BOOL)(ao==G->WR[wrwin]->GUI.TE_EDIT);
   }
   return FALSE;
}

//  Dispatcher for timer class (called once every second)
void TC_Dispatcher(void)
{
   if (CheckIO((struct IORequest *)TCData.req))
   {
      int i;
      WaitIO((struct IORequest *)TCData.req);
      if (++G->SI_Count >= C->WriteIndexes && C->WriteIndexes)
         if (!TC_ActiveEditor(0) && !TC_ActiveEditor(1))
         {
            MA_UpdateIndexes(FALSE);
            G->SI_Count = 0;
         }
      if (++G->GM_Count >= C->CheckMailDelay*60 && C->CheckMailDelay)
      {
         for (i = 0; i < MAXWR; i++) if (G->WR[i]) break;
         if (i == MAXWR && !G->CO)
         {
            MA_PopNow(POP_TIMED,-1);
            G->GM_Count = 0;
         }
      }
      for (i = 0; i < MAXWR; i++) if (G->WR[i]) if (++G->WR[i]->AS_Count >= C->AutoSave && C->AutoSave)
      {
         EditorToFile(G->WR[i]->GUI.TE_EDIT, WR_AutoSaveFile(i), NULL);
         G->WR[i]->AS_Count = 0;
         G->WR[i]->AS_Done = TRUE;
      }
      TC_Start();
   }
}
///
/// AY_PrintStatus
//  Shows progress of program initialization
void AY_PrintStatus(char *txt, int percent)
{
   set(G->AY_Text, MUIA_Gauge_InfoText, txt);
   set(G->AY_Text, MUIA_Gauge_Current, percent);
   DoMethod(G->App, MUIM_Application_InputBuffered);
}
///
/// AY_SendMailFunc
//  User clicked e-mail URL in About window
void SAVEDS AY_SendMailFunc(void)
{
   int wrwin;
   if (G->MA) if ((wrwin = MA_NewNew(NULL, 0)) >= 0)
   {
      setstring(G->WR[wrwin]->GUI.ST_TO, "YAM Support <support@yam.ch>");
      set(G->WR[wrwin]->GUI.WI, MUIA_Window_ActiveObject, G->WR[wrwin]->GUI.ST_SUBJECT);
   }
}
MakeHook(AY_SendMailHook, AY_SendMailFunc);
///
/// AY_GoPageFunc
//  User clicked homepage URL in About window
void SAVEDS AY_GoPageFunc(void)
{
   GotoURL("http://www.yam.ch/");
}
MakeHook(AY_GoPageHook, AY_GoPageFunc);
///
/// AY_New
//  Creates About window
BOOL AY_New(BOOL hidden)
{
   char logopath[SIZE_PATHFILE];
   APTR ft_text, bt_sendmail, bt_gopage;
   struct DateTime dt;
   char datebuf[LEN_DATSTRING];

   dt.dat_Stamp.ds_Days   = __YAM_VERDAYS;
   dt.dat_Stamp.ds_Minute = 0;
   dt.dat_Stamp.ds_Tick   = 0;
   dt.dat_Format  = FORMAT_DOS;
   dt.dat_Flags   = 0L;
   dt.dat_StrDay  = NULL;
   dt.dat_StrDate = datebuf;
   dt.dat_StrTime = NULL;
   DateToStr(&dt);

   strmfp(logopath, G->ProgDir, "Icons/logo");
   G->AY_Win = WindowObject,
      MUIA_Window_Title, GetStr(MSG_MA_About),
      MUIA_Window_ID, MAKE_ID('C','O','P','Y'),
      MUIA_Window_Activate, FALSE,
      MUIA_HelpNode, "COPY",
      WindowContents, VGroup,
         MUIA_Background, MUII_GroupBack,
         Child, HGroup,
            MUIA_Group_Spacing, 0,
            Child, HSpace(0),
            Child, NewObject(CL_BodyChunk->mcc_Class,NULL,
               MUIA_Bodychunk_File, logopath,
            End,
            Child, HSpace(0),
         End,
         Child, HCenter((VGroup,
            Child, CLabel(GetStr(MSG_Copyright1)),
            Child, ColGroup(2),
               Child, bt_sendmail = TextObject,
                  MUIA_Text_Contents, "\033c\033u\0335support@yam.ch",
                  MUIA_InputMode, MUIV_InputMode_RelVerify,
               End,
               Child, bt_gopage = TextObject,
                  MUIA_Text_Contents, "\033c\033u\0335http://www.yam.ch/",
                  MUIA_InputMode, MUIV_InputMode_RelVerify,
               End,
            End,
            Child, RectangleObject,
               MUIA_Rectangle_HBar, TRUE,
               MUIA_FixHeight, 8,
            End,
            Child, ColGroup(2),
               MUIA_Group_HorizSpacing, 8,
               MUIA_Group_VertSpacing, 2,
               Child, Label(GetStr(MSG_Version)),
               Child, LLabel(__YAM_VERSION),
               Child, Label(GetStr(MSG_CompilationDate)),
               Child, LLabel(datebuf),
             End,
         End)),
         Child, G->AY_Group = PageGroup,
            Child, ft_text = FloattextObject,
            	GroupFrame,
            End,
            Child, ScrollgroupObject,
               MUIA_Scrollgroup_FreeHoriz, FALSE,
               MUIA_Scrollgroup_Contents, VGroupV,
                  GroupFrame,
                  Child, G->AY_List = VGroup,
                     Child, TextObject,
                        MUIA_Text_Contents, GetStr(MSG_UserLogin),
                        MUIA_Background,		MUII_TextBack,
                        MUIA_Frame,					MUIV_Frame_Text,
                        MUIA_Text_PreParse, MUIX_C MUIX_PH,
                     End,
                  End,
                  Child, HVSpace,
               End,
            End,
         End,
         Child, G->AY_Text = GaugeObject,
            GaugeFrame,
            MUIA_Gauge_InfoText, " ",
            MUIA_Gauge_Horiz, TRUE,
         End,
         Child, G->AY_Button = TextObject,
         	 MUIA_ShowMe,				 FALSE,
	         MUIA_Text_Contents, GetStr(MSG_CO_PhraseClose),
           MUIA_Background,    MUII_ButtonBack,
           MUIA_Frame,         MUIV_Frame_Button,
           MUIA_InputMode,     MUIV_InputMode_RelVerify,
           MUIA_Text_SetMax,   TRUE,
           MUIA_CycleChain,    1,
         End,
      End,
   End;

   /* if the WindowObject could be created */
   if (G->AY_Win)
   {
    	/* now we create the about text */
      G->AY_AboutText = AllocStrBuf(SIZE_LARGE);
      G->AY_AboutText = StrBufCat(G->AY_AboutText, GetStr(MSG_Copyright2));
      G->AY_AboutText = StrBufCat(G->AY_AboutText, GetStr(MSG_UsedSoftware));
      G->AY_AboutText = StrBufCat(G->AY_AboutText, 	"\0338Magic User Interface\0332 (Stefan Stuntz)\n"
                     						        						"\0338TextEditor.mcc, BetterString.mcc\0332 (Allan Odgaard)\n"
												                            "\0338Toolbar.mcc\0332 (Benny Kjær Nielsen)\n"
            											                  "\0338NListtree.mcc\0332 (Carsten Scholling)\n"
                       												      "\0338NList.mcc, NListview.mcc\0332 (Gilles Masson)\n"
					                          						    "\0338XPK\0332 (Urban D. Müller, Dirk Stöcker)\n"
                                                    "\0338popupmenu.library (Henrik Isaksson)\n\n");
      G->AY_AboutText = StrBufCat(G->AY_AboutText, GetStr(MSG_WebSite));
      set(ft_text, MUIA_Floattext_Text, G->AY_AboutText);

      DoMethod(G->App, OM_ADDMEMBER, G->AY_Win);
      DoMethod(bt_sendmail,MUIM_Notify,MUIA_Pressed,            FALSE,MUIV_Notify_Application,2,MUIM_CallHook,&AY_SendMailHook);
      DoMethod(bt_gopage  ,MUIM_Notify,MUIA_Pressed,            FALSE,MUIV_Notify_Application,2,MUIM_CallHook,&AY_GoPageHook);
      DoMethod(G->AY_Win  ,MUIM_Notify,MUIA_Window_CloseRequest,TRUE ,G->AY_Win              ,3,MUIM_Set,MUIA_Window_Open, FALSE);

      // If the close button will be pressed we close the window
      DoMethod(G->AY_Button,  MUIM_Notify, MUIA_Pressed, FALSE,	G->AY_Win, 3, MUIM_Set, MUIA_Window_Open, FALSE, TAG_DONE);

      set(G->AY_Win, MUIA_Window_Open, !hidden);

      return TRUE;
   }
   return FALSE;
}
///
/// PopUp
//  Un-iconify YAM
void PopUp(void)
{
   int winopen;
   nnset(G->App, MUIA_Application_Iconified,FALSE);
   get(G->MA->GUI.WI, MUIA_Window_Open, &winopen);
   if (!winopen) set(G->MA->GUI.WI, MUIA_Window_Open, TRUE);
   DoMethod(G->MA->GUI.WI, MUIM_Window_ScreenToFront);
}
///
/// DoublestartHook
//  A second copy of YAM was started
void SAVEDS DoublestartFunc(void)
{
//   PopUp();
//   ^^^^^^^^ Crap! If we want to popup the other (running) YAM,
//            we can't use our own app object (NULL) pointer!
}
MakeHook(DoublestartHook, DoublestartFunc);
///
/// StayInProg
//  Makes sure that the user really wants to quit the program
BOOL StayInProg(void)
{
   BOOL req = FALSE;
   int i, fq;

   if (G->AB->Modified)
   {
      if (MUI_Request(G->App, G->MA->GUI.WI, 0, NULL, GetStr(MSG_MA_ABookModifiedGad), GetStr(MSG_AB_Modified)))
         AB_SaveABookFunc();
   }
   if (G->CO || C->ConfirmOnQuit) req = TRUE;
   for (i = 0; i < 4; i++) if (G->EA[i]) req = TRUE;
   for (i = 0; i < 2; i++) if (G->WR[i]) req = TRUE;
   get(G->App, MUIA_Application_ForceQuit, &fq);
   if (fq) req = FALSE;
   if (!req) return FALSE;
   return (BOOL)!MUI_Request(G->App, G->MA->GUI.WI, 0, GetStr(MSG_MA_ConfirmReq), GetStr(MSG_YesNoReq), GetStr(MSG_QuitYAMReq));
}
///
/// Root_GlobalDispatcher
//  Processes return value of MUI_Application_NewInput
BOOL Root_GlobalDispatcher(ULONG app_input)
{
   switch (app_input)
   {
      case MUIV_Application_ReturnID_Quit: return (BOOL)!StayInProg();
      case ID_CLOSEALL: if (!C->IconifyOnQuit) return (BOOL)!StayInProg();
                        set(G->App, MUIA_Application_Iconified, TRUE); break;
      case ID_RESTART:  return 2;
      case ID_ICONIFY:  MA_UpdateIndexes(FALSE); break;
   }
   return FALSE;
}
///
/// Root_New
//  Creates MUI application
BOOL Root_New(BOOL hidden)
{
#define MUIA_Application_UsedClasses 0x8042e9a7
   static char *classes[] = { "TextEditor.mcc", "Toolbar.mcc", "BetterString.mcc", "InfoText.mcc", "NListtree.mcc", "NList.mcc", "NListview.mcc", NULL };
   G->App = ApplicationObject,
      MUIA_Application_Author     ,"YAM Open Source Team",
      MUIA_Application_Base       ,"YAM",
      MUIA_Application_Title      ,"YAM",
      MUIA_Application_Version    ,"$VER: YAM " __YAM_VERSION " (" __YAM_VERDATE ")",
      MUIA_Application_Copyright  ,"© 2000-2001 by YAM Open Source Team",
      MUIA_Application_Description,GetStr(MSG_AppDescription),
      MUIA_Application_UseRexx    ,FALSE,
      MUIA_Application_SingleTask ,!getenv("MultipleYAM"),
      MUIA_Application_UsedClasses, classes,
   End;
   if (G->App)
   {
      set(G->App, MUIA_Application_HelpFile, "YAM.guide");
      set(G->App, MUIA_Application_Iconified, hidden);
      DoMethod(G->App, MUIM_Notify, MUIA_Application_DoubleStart, TRUE, MUIV_Notify_Application, 2, MUIM_CallHook, &DoublestartHook);
      DoMethod(G->App, MUIM_Notify, MUIA_Application_Iconified, TRUE, MUIV_Notify_Application, 2, MUIM_Application_ReturnID, ID_ICONIFY);
      if (AY_New(hidden)) return TRUE;
   }
   return FALSE;
}
///

/// Terminate
//  Deallocates used memory and MUI modules and terminates
void Terminate(void)
{
   int i;
 	 struct Folder **flist;

   if (G->CO) { CO_FreeConfig(CE); free(CE); DisposeModule(&G->CO); }
   for (i = 0; i < MAXEA; i++) DisposeModule(&G->EA[i]);
   for (i = 0; i < MAXRE; i++) if (G->RE[i]) { RE_CleanupMessage(i); DisposeModule(&G->RE[i]); }
   for (i = 0; i <=MAXWR; i++) if (G->WR[i]) { WR_Cleanup(i); DisposeModule(&G->WR[i]); }
   if (G->TR) { TR_Cleanup(); TR_CloseTCPIP(); }
   DisposeModule(&G->FO);
   DisposeModule(&G->FI);
   DisposeModule(&G->ER);
   DisposeModule(&G->US);
   if (G->MA)
   {
      MA_UpdateIndexes(FALSE);
      G->Weights[0] = GetMUI(G->MA->GUI.LV_FOLDERS, MUIA_HorizWeight);
      G->Weights[1] = GetMUI(G->MA->GUI.LV_MAILS, MUIA_HorizWeight);
      SaveLayout(TRUE);
      set(G->MA->GUI.WI, MUIA_Window_Open, FALSE);

      // lets free every folder resource
      if ((flist = FO_CreateList()))
      {
         for (i = 1; i <= (int)*flist; i++)
         {
           FO_FreeFolder(flist[i]);
         }

         free(flist);
      }
   }
   DisposeModule(&G->AB);
   DisposeModule(&G->MA);
   if (G->TTin) free(G->TTin);
   if (G->TTout) free(G->TTout);
   for (i = 0; i < MAXASL; i++) if (G->ASLReq[i]) MUI_FreeAslRequest(G->ASLReq[i]);
   for (i = 0; i < MAXWR+1; i++) if (G->WR_NRequest[i].nr_stuff.nr_Msg.nr_Port) DeletePort(G->WR_NRequest[i].nr_stuff.nr_Msg.nr_Port);
   if (G->AppIcon) RemoveAppIcon(G->AppIcon);
   if (G->AppPort) DeletePort(G->AppPort);
   if (G->RexxHost) CloseDownARexxHost(G->RexxHost);
   TC_Exit();
   if (G->AY_AboutText) FreeStrBuf(G->AY_AboutText);
   if (G->App) MUI_DisposeObject(G->App);
   for (i = 0; i < MAXICONS; i++) if (G->DiskObj[i]) FreeDiskObject(G->DiskObj[i]);
   for (i = 0; i < MAXIMAGES; i++) if (G->BImage[i]) FreeBCImage(G->BImage[i]);
   CO_FreeConfig(C);
   ExitClasses();
   if (DataTypesBase) CloseLibrary(DataTypesBase);
   if (XpkBase)       CloseLibrary(XpkBase);
   if (MUIMasterBase) CloseLibrary(MUIMasterBase);
   CloseYAMCatalog();
   if (G->Locale) CloseLocale(G->Locale);
   if (LocaleBase) CloseLibrary((struct Library *)LocaleBase);
   if (WorkbenchBase) CloseLibrary(WorkbenchBase);
   if (IconBase) CloseLibrary(IconBase);
	 if (PopupMenuBase) CloseLibrary((struct Library *)PopupMenuBase);
#ifdef __ixemul__
   if (RexxSysBase) CloseLibrary(RexxSysBase);
#endif
   if (IFFParseBase) CloseLibrary(IFFParseBase);
   if (KeymapBase) CloseLibrary(KeymapBase);
   if (IntuitionBase) CloseLibrary((struct Library *) IntuitionBase);
   if (UtilityBase) CloseLibrary((struct Library *)UtilityBase);
   free(C); free(G);
   if (yamLast) exit(0);
}
///
/// Abort
//  Shows error requester, then terminates the program
void Abort(char *error)
{
   if (error)
   {
      if (MUIMasterBase) {
         MUI_Request(G->App ? G->App : NULL, NULL, 0, GetStr(MSG_ErrorStartup), GetStr(MSG_Quit), error);
      }
      else if (IntuitionBase) {
         struct EasyStruct ErrReq = {
            sizeof (struct EasyStruct),
            0,
            NULL,
            NULL,
            NULL
         };

         ErrReq.es_Title        = (char *)GetStr(MSG_ErrorStartup);
         ErrReq.es_TextFormat   = error;
         ErrReq.es_GadgetFormat = (char *)GetStr(MSG_Quit);

         EasyRequest(NULL, &ErrReq, NULL, error);
      }
      else puts(error);
   }
   yamLast = TRUE;
   Terminate();
}
///
/// CheckMCC
//  Checks if a certain version of a MCC is available
BOOL CheckMCC(char *name, int minver, int minrev, BOOL req)
{
   Object *obj;
   BOOL success = FALSE;

   obj = MUI_NewObject(name, TAG_DONE);
   if (obj) {
      ULONG tmp;
      int ver, rev;

      get(obj, MUIA_Version, &tmp);
      ver = (int)tmp;
      get(obj, MUIA_Revision, &tmp);
      rev = (int)tmp;

      if (ver > minver || (ver == minver && rev >= minrev)) {
         success = TRUE;
      }
      MUI_DisposeObject(obj);
   }

   if (!success && req) {
      static char errorlib[SIZE_DEFAULT];
      sprintf(errorlib, GetStr(MSG_ErrorLib), name, minver);
      Abort(errorlib);
   }

   return success;
}
///
/// InitLib
//  Opens a library
struct Library *InitLib(char *libname, int version, int revision, BOOL required, BOOL close)
{
   struct Library *lib = OpenLibrary(libname, version);
   if (lib && revision) if (lib->lib_Version == version && lib->lib_Revision < revision) { CloseLibrary(lib); lib = NULL; }
   if (!lib && required)
   {
      static char errorlib[SIZE_DEFAULT];
      sprintf(errorlib, GetStr(MSG_ErrorLib), libname, version);
      Abort(errorlib);
   }
   if (lib && close) CloseLibrary(lib);
   return lib;
}
///
/// SetupAppIcons
//  Sets location of mailbox status icon on workbench screen
void SetupAppIcons(void)
{
   int i;
   for (i = 0; i < 4; i++) if (G->DiskObj[i])
   {
      G->DiskObj[i]->do_CurrentX = C->IconPositionX;
      G->DiskObj[i]->do_CurrentY = C->IconPositionY;
   }
}
///
/// Initialise2
//  Phase 2 of program initialization (after user logs in)
void Initialise2(BOOL hidden)
{
   BOOL newfolders = FALSE;
   int i;
   struct Folder *folder, **oldfolders = NULL;

   AY_PrintStatus(GetStr(MSG_LoadingConfig), 30);
   CO_SetDefaults(C, -1);
   CO_LoadConfig(C, G->CO_PrefsFile, &oldfolders);
   CO_Validate(C, FALSE);
   AY_PrintStatus(GetStr(MSG_CreatingGUI), 40);
   if (!(G->MA = MA_New()) || !(G->AB = AB_New())) Abort(GetStr(MSG_ErrorMuiApp));
   MA_SetupDynamicMenus();
   MA_ChangeSelectedFunc();
   SetupAppIcons();
   LoadLayout();
   set(G->MA->GUI.LV_FOLDERS, MUIA_HorizWeight, G->Weights[0]);
   set(G->MA->GUI.LV_MAILS,   MUIA_HorizWeight, G->Weights[1]);
   AY_PrintStatus(GetStr(MSG_LoadingFolders), 50);
   if (!FO_LoadTree(CreateFilename(".folders")) && oldfolders)
   {
      for (i = 0; i < 100; i++) if (oldfolders[i]) DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NListtree_Insert, oldfolders[i]->Name, oldfolders[i], MUIV_NListtree_Insert_ListNode_Root, TAG_DONE);
      newfolders = TRUE;
   }
   if (oldfolders) { for (i = 0; oldfolders[i]; i++) free(oldfolders[i]); free(oldfolders); }
   if (!FO_GetFolderByType(FT_INCOMING,NULL)) newfolders |= FO_CreateFolder(FT_INCOMING, FolderNames[0], GetStr(MSG_MA_Incoming));
   if (!FO_GetFolderByType(FT_OUTGOING,NULL)) newfolders |= FO_CreateFolder(FT_OUTGOING, FolderNames[1], GetStr(MSG_MA_Outgoing));
   if (!FO_GetFolderByType(FT_SENT    ,NULL)) newfolders |= FO_CreateFolder(FT_SENT    , FolderNames[2], GetStr(MSG_MA_Sent));
   if (!FO_GetFolderByType(FT_DELETED ,NULL)) newfolders |= FO_CreateFolder(FT_DELETED , FolderNames[3], GetStr(MSG_MA_Deleted));
   if (newfolders) FO_SaveTree(CreateFilename(".folders"));
   AY_PrintStatus(GetStr(MSG_RebuildIndices), 60);
   MA_UpdateIndexes(TRUE);
   AY_PrintStatus(GetStr(MSG_LoadingFolders), 75);
   for (i = 0; ; i++)
   {
    	struct MUI_NListtree_TreeNode *tn = (struct MUI_NListtree_TreeNode *)DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NListtree_GetEntry, MUIV_NListtree_GetEntry_ListNode_Root, i, MUIV_NListtree_GetEntry_Flag_Visible, TAG_DONE);
      if (!tn) break;
      folder = tn->tn_User;
      if (!folder) break;
      if ((folder->Type == FT_INCOMING || folder->Type == FT_OUTGOING || folder->Type == FT_DELETED || C->LoadAllFolders) && !(folder->XPKType&1)) MA_GetIndex(folder);
      else if (folder->Type != FT_GROUP) folder->LoadedMode = MA_LoadIndex(folder, FALSE);
      DoMethod(G->App, MUIM_Application_InputBuffered);
   }
   G->NewMsgs = -1;
   MA_ChangeFolder(FO_GetFolderByType(FT_INCOMING, NULL));
   AY_PrintStatus(GetStr(MSG_LoadingABook), 90);
   AB_LoadTree(G->AB_Filename, FALSE, FALSE);
   if (!(G->RexxHost = SetupARexxHost("YAM", NULL))) Abort(GetStr(MSG_ErrorARexx));
   AY_PrintStatus("", 100);
   set(G->MA->GUI.WI, MUIA_Window_Open, !hidden);
   set(G->AY_Win, MUIA_Window_Open, FALSE);
   set(G->AY_Text, MUIA_ShowMe, FALSE);
	 set(G->AY_Button, MUIA_ShowMe, TRUE);
}
///
/// Initialise
//  Phase 1 of program initialization (before user logs in)
void Initialise(BOOL hidden)
{
   static char iconfile[SIZE_PATHFILE];
   char iconpath[SIZE_PATH];
   char *icnames[MAXICONS] = { "empty", "old", "new", "check" };
   char *imnames[MAXIMAGES] = { "status_unread", "status_old", "status_forward",
      "status_reply", "status_waitsend", "status_error", "status_hold",
      "status_sent", "status_new", "status_delete", "status_download",
      "status_group", "status_urgent", "status_attach", "status_report",
      "status_crypt", "status_signed" };
   int i;

   DateStamp(&G->StartDate);

   /* First open locale.library, so we can display a translated error requester
      in case some of the other libraries can't be opened. */
   if ((LocaleBase = (struct LocaleBase *)InitLib("locale.library", 38, 0, TRUE, FALSE))) G->Locale = OpenLocale(NULL);
   OpenYAMCatalog();

   UtilityBase = (struct UtilityBase *)InitLib("utility.library", 36, 0, TRUE, FALSE);
   KeymapBase = InitLib("keymap.library", 36, 0, TRUE, FALSE);
   IFFParseBase = InitLib("iffparse.library", 36, 0, TRUE, FALSE);
#ifdef __ixemul__
   RexxSysBase = InitLib("rexxsyslib.library", 36, 0, TRUE, FALSE);
#endif
   MUIMasterBase = InitLib("muimaster.library", 19, 0, TRUE, FALSE);

	 // we open the popupmenu.library for the ContextMenus in YAM but it`s not a MUST.
   PopupMenuBase = (struct PopupMenuBase *)InitLib(POPUPMENU_NAME, 9, 0, FALSE, FALSE);

   /* We can't use CheckMCC() due to a bug in Toolbar.mcc! */
   InitLib("mui/Toolbar.mcc", 15, 6, TRUE, TRUE);

   CheckMCC(MUIC_NListtree, 18, 0, TRUE);

   if (!InitClasses()) Abort(GetStr(MSG_ErrorClasses));
   if (!Root_New(hidden)) Abort(FindPort("YAM") ? NULL : GetStr(MSG_ErrorMuiApp));
   AY_PrintStatus(GetStr(MSG_InitLibs), 10);
   XpkBase = InitLib(XPKNAME, 0, 0, FALSE, FALSE);
   if ((DataTypesBase = InitLib("datatypes.library", 39, 0, FALSE, FALSE)))
      if (CheckMCC("Dtpic.mui", 0, 0, FALSE)) G->DtpicSupported = TRUE;
   if (!TC_Init()) Abort(GetStr(MSG_ErrorTimer));
   for (i = 0; i < MAXASL; i++)
      if (!(G->ASLReq[i] = MUI_AllocAslRequestTags(ASL_FileRequest, ASLFR_RejectIcons, TRUE,
         TAG_END))) Abort(GetStr(MSG_ErrorAslStruct));
   G->AppPort = CreatePort(NULL, 0);
   for (i = 0; i < 3; i++)
   {
      G->WR_NRequest[i].nr_stuff.nr_Msg.nr_Port = CreatePort(NULL, 0);
      G->WR_NRequest[i].nr_Name = (UBYTE *)G->WR_Filename[i];
      G->WR_NRequest[i].nr_Flags = NRF_SEND_MESSAGE;
   }
   srand(GetDateStamp());
   AY_PrintStatus(GetStr(MSG_LoadingGFX), 20);
   strmfp(iconpath, G->ProgDir, "Icons");
   for (i = 0; i < MAXICONS; i++)
   {
      strmfp(iconfile, iconpath, icnames[i]);
      G->DiskObj[i] = GetDiskObject(iconfile);
   }
   for (i = 0; i < MAXIMAGES; i++)
   {
      strmfp(iconfile, iconpath, imnames[i]);
      G->BImage[i] = LoadBCImage(iconfile);
      DoMethod(G->App, MUIM_Application_InputBuffered);
   }
}
///
/// SendWaitingMail
//  Sends pending mail on startup
void SendWaitingMail(void)
{
   struct Mail *mail;
   BOOL doit = TRUE;
   int tots = 0, hidden;
   struct Folder *fo = FO_GetFolderByType(FT_OUTGOING, NULL);

   get(G->App, MUIA_Application_Iconified, &hidden);
   for (mail = fo->Messages; mail; mail = mail->Next) if (mail->Status != STATUS_HLD) tots++;
   if (!tots) return;
   MA_ChangeFolder(fo);
   if (!hidden) doit = MUI_Request(G->App, G->MA->GUI.WI, 0, NULL, GetStr(MSG_YesNoReq), GetStr(MSG_SendStartReq));
   if (doit) MA_Send(SEND_ALL);
}
///
/// DoStartup
//  Performs different checks/cleanup operations on startup
void DoStartup(BOOL nocheck, BOOL hide)
{

   if (C->CleanupOnStartup) DoMethod(G->App, MUIM_CallHook, &MA_DeleteOldHook);
   if (C->RemoveOnStartup) DoMethod(G->App, MUIM_CallHook, &MA_DeleteDeletedHook);
   if (C->CheckBirthdates && !nocheck && !hide) AB_CheckBirthdates();
   if (TR_IsOnline())
   {
      if (C->GetOnStartup && !nocheck) {
         MA_PopNow(POP_START,-1);
         if (G->TR) DisposeModule(&G->TR);
      }
      if (C->SendOnStartup && !nocheck) {
         SendWaitingMail();
         if (G->TR) DisposeModule(&G->TR);
      }
   }
}
///
/// Login
//  Allows automatical login for AmiTCP-Genesis users
#ifdef __STORM__
struct Library *GenesisBase;
#endif
void Login(char *user, char *password, char *maildir, char *prefsfile)
{
#ifndef __STORM__
   struct Library *GenesisBase;
#endif
   struct genUser *guser;
   BOOL terminate = FALSE, loggedin = FALSE;

   if ((GenesisBase = OpenLibrary("genesis.library", 1)))
   {
      if ((guser = GetGlobalUser()))
      {
         terminate = !(loggedin = US_Login(guser->us_name, "\01", maildir, prefsfile));
         FreeUser(guser);
      }
      CloseLibrary(GenesisBase);
   }
   if (!loggedin && !terminate) terminate = !US_Login(user, password, maildir, prefsfile);
   if (terminate) Abort(NULL);
}
///
/// GetDST
//  Checks if daylight saving time is active
int GetDST(void)
{
   int i;
   char *dst;

   if((dst = getenv("IXGMTOFFSET")))
   {
      return dst[4]?2:1;
   }

   dst = getenv("SUMMERTIME");
   if (!dst) return 0;
   for (i = 0; i < 11; i++)
   {
      while (*dst != ':') if (!*dst++) return 0;
      dst++;
   }
   return (*dst == 'Y' ? 2 : 1);
}
///

/// Main
//  Program entry point, main loop
#ifdef __GNUC__
void STACKEXT main(int argc, char **argv)
#else
void main(int argc, char **argv)
#endif
{
   struct NewRDArgs nrda;
   struct { STRPTR user;
            STRPTR password;
            STRPTR maildir;
            STRPTR prefsfile;
            LONG   nocheck;
            LONG   hide;
            LONG   debug;
            STRPTR mailto;
            STRPTR subject;
            STRPTR letter;
            STRPTR *attach;
          } args = { NULL, NULL, NULL, NULL, FALSE, FALSE, FALSE, NULL, NULL, NULL, NULL };
   int wrwin, err, ret;
   char **sptr, progdir[SIZE_PATH];
   ULONG signals, appsigs, timsigs, notsigs0, notsigs1, notsigs2, rexsigs;
   struct Message *msg;
   struct User *user;
   BPTR progdirlock, yamlock, oldcdirlock;

   IntuitionBase = (struct IntuitionBase *) InitLib("intuition.library", 36, 0, TRUE, FALSE);
   IconBase = InitLib("icon.library", 36, 0, TRUE, FALSE);
   WorkbenchBase = InitLib("workbench.library", 36, 0, TRUE, FALSE);

#ifdef __ixemul__
   if(0 == argc) _WBenchMsg=(struct WBStartup *)argv;
#endif

   nrda.Template = "USER/K,PASSWORD/K,MAILDIR/K,PREFSFILE/K,NOCHECK/S,HIDE/S,DEBUG/S,MAILTO/K,SUBJECT/K,LETTER/K,ATTACH/M";
   nrda.ExtHelp = NULL;
   nrda.Window = NULL;
   nrda.Parameters = (LONG *)&args;
   nrda.FileParameter = -1;
   nrda.PrgToolTypesOnly = FALSE;
   if ((err = NewReadArgs(_WBenchMsg, &nrda)))
   {
      PrintFault(err, "YAM");
      NewFreeArgs(&nrda);
      if (WorkbenchBase) CloseLibrary(WorkbenchBase);
      if (IconBase) CloseLibrary(IconBase);
      if (IntuitionBase) CloseLibrary((struct Library *) IntuitionBase);
      exit(5);
   }
   if ((progdirlock = GetProgramDir()))
      NameFromLock(progdirlock, progdir, SIZE_PATH);
   else
   {
      strcpy(progdir, "YAM:");
      SetProgramDir(Lock(progdir, ACCESS_READ));
   }
   yamlock = Lock(progdir, ACCESS_READ);
   oldcdirlock = CurrentDir(yamlock);
   while (1)
   {
      G = calloc(1,sizeof(struct Global));
      C = calloc(1,sizeof(struct Config));
      strcpy(G->ProgDir, progdir);
      if (!args.maildir) strcpy(G->MA_MailDir, progdir);
      args.hide = -args.hide; args.nocheck = -args.nocheck;
      G->TR_Debug = -args.debug;
      G->TR_Allow = TRUE;
      G->CO_DST = GetDST();
      if (yamFirst)
      {
         Initialise(args.hide);
         Login(args.user, args.password, args.maildir, args.prefsfile);
         Initialise2(args.hide);
         DoMethod(G->App, MUIM_Application_Load, MUIV_Application_Load_ENVARC);
         AppendLog(0, GetStr(MSG_LOG_Started), "", "", "", "");
         MA_StartMacro(MACRO_STARTUP, NULL);
         DoStartup(args.nocheck, args.hide);
         if (args.mailto || args.letter || args.subject || args.attach) if ((wrwin = MA_NewNew(NULL, 0)) >= 0)
         {
            if (args.mailto) setstring(G->WR[wrwin]->GUI.ST_TO, args.mailto);
            if (args.subject) setstring(G->WR[wrwin]->GUI.ST_SUBJECT, args.subject);
            if (args.letter) FileToEditor(args.letter, G->WR[wrwin]->GUI.TE_EDIT);
            if (args.attach) for (sptr = (char**)args.attach; *sptr; sptr++)
               if (FileSize(*sptr) >= 0) WR_AddFileToList(wrwin, *sptr, NULL, FALSE);
         }
         yamFirst = FALSE;
      }
      else
      {
         Initialise(FALSE);
         Login(NULL, NULL, NULL, NULL);
         Initialise2(FALSE);
         DoMethod(G->App, MUIM_Application_Load, MUIV_Application_Load_ENVARC);
      }
      user = US_GetCurrentUser();
      AppendLogNormal(1, GetStr(MSG_LOG_LoggedIn), user->Name, "", "", "");
      AppendLogVerbose(1, GetStr(MSG_LOG_LoggedInVerbose), user->Name, G->CO_PrefsFile, G->MA_MailDir, "");
      TC_Start();
      appsigs  = 1<<G->AppPort->mp_SigBit;
      timsigs  = 1<<TCData.port->mp_SigBit;
      notsigs0 = 1<<G->WR_NRequest[0].nr_stuff.nr_Msg.nr_Port->mp_SigBit;
      notsigs1 = 1<<G->WR_NRequest[1].nr_stuff.nr_Msg.nr_Port->mp_SigBit;
      notsigs2 = 1<<G->WR_NRequest[2].nr_stuff.nr_Msg.nr_Port->mp_SigBit;
      rexsigs  = 1<<G->RexxHost->port->mp_SigBit;
      while (!(ret = Root_GlobalDispatcher(DoMethod(G->App, MUIM_Application_NewInput, &signals))))
      {
         if (signals)
         {
            signals = Wait(signals | timsigs | SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_F | appsigs | notsigs0 | notsigs1 | rexsigs);
            if (signals & SIGBREAKF_CTRL_C) break;
            if (signals & SIGBREAKF_CTRL_F) PopUp();
            if (signals & timsigs) TC_Dispatcher();
            if (signals & rexsigs) ARexxDispatch(G->RexxHost);
            if (signals & appsigs)
            {
               struct AppMessage *apmsg;
               while ((apmsg = (struct AppMessage *)GetMsg(G->AppPort)))
               {
                  if (apmsg->am_Type == AMTYPE_APPICON)
                  {
                     PopUp();
                     if (apmsg->am_NumArgs)
                     {
                        int wrwin;
                        if      (G->WR[0]) wrwin = 0;
                        else if (G->WR[1]) wrwin = 1;
                        else wrwin = MA_NewNew(NULL, 0);
                        if (wrwin >= 0) WR_App(wrwin, apmsg);
                     }
                  }
                  ReplyMsg((struct Message *)apmsg);
               }
            }
            if (signals & notsigs0)
            {
               while ((msg = GetMsg(G->WR_NRequest[0].nr_stuff.nr_Msg.nr_Port))) ReplyMsg(msg);
               if (G->WR[0]) FileToEditor(G->WR_Filename[0], G->WR[0]->GUI.TE_EDIT);
            }
            if (signals & notsigs1)
            {
               while ((msg = GetMsg(G->WR_NRequest[1].nr_stuff.nr_Msg.nr_Port))) ReplyMsg(msg);
               if (G->WR[1]) FileToEditor(G->WR_Filename[1], G->WR[1]->GUI.TE_EDIT);
            }
            if (signals & notsigs2)
            {
               while ((msg = GetMsg(G->WR_NRequest[2].nr_stuff.nr_Msg.nr_Port))) ReplyMsg(msg);
               if (G->WR[2]) FileToEditor(G->WR_Filename[2], G->WR[2]->GUI.TE_EDIT);
            }
         }
      }
      if (C->SendOnQuit && !args.nocheck) if (TR_IsOnline()) SendWaitingMail();
      if (C->CleanupOnQuit) DoMethod(G->App, MUIM_CallHook, &MA_DeleteOldHook);
      if (C->RemoveOnQuit) DoMethod(G->App, MUIM_CallHook, &MA_DeleteDeletedHook);
      if (ret == 1)
      {
         yamLast = TRUE;
         AppendLog(99, GetStr(MSG_LOG_Terminated), "", "", "", "");
         MA_StartMacro(MACRO_QUIT, NULL);
         CurrentDir(oldcdirlock);
         UnLock(yamlock);
         NewFreeArgs(&nrda);
      }
      FreeData2D(&Header);
      Terminate();
   }
}
///

