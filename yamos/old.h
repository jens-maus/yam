#if defined(__SASC)
  #if !defined(_M68060)
    #if !defined(_M68040)
      #if !defined(_M68030) && !defined(_M68020)
         #define __mc68000__
      #else
        #define __mc68020__
      #endif
    #else
      #define __mc68040__
    #endif
  #else
    #define __mc68060__
  #endif
  #if defined(_M68881)
    #define __HAVE_68881__
  #endif
#endif

#if defined(__mc68020__) || defined(__mc68030__) || defined(__mc68040__) || defined(__mc68060__)
  #define PLAIN(x)
  #define REQUIRES_68020(x) ((x & AFF_68020) == 0)
#else
  #define REQUIRES_68020(x) (0)
  #define PLAIN(x) x
#endif

/*
** stacksize definitions
*/

/* we need to resolve the __near problem directly instead modifiing compiler.h */

#ifdef __STORMGCC__
  #define __YAM_STACK __stacksize
#else
  #define __YAM_STACK __stack
#endif

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

#include <exec/memory.h>
#include <exec/execbase.h>
#include <dos/datetime.h>
#include <dos/dostags.h>
#include <dos/doshunks.h>
#include <workbench/workbench.h>
#include <workbench/startup.h>
#include <devices/printer.h>
#include <intuition/icclass.h>
#include <intuition/gadgetclass.h>
#include <datatypes/pictureclass.h>
#include <datatypes/soundclass.h>
#include <libraries/locale.h>
#include <libraries/asl.h>
#include <libraries/mui.h>
#include <libraries/gadtools.h>
#include <libraries/openurl.h>
#include <libraries/genesis.h>
#include <libraries/cmanager.h>
#include <mui/NListtree_mcc.h>
#include <mui/NList_mcc.h>
#include <mui/NListview_mcc.h>
#include <mui/TextEditor_mcc.h>
#include <mui/BetterString_mcc.h>
#include <mui/Toolbar_mcc.h>
#include <rexx/rxslib.h>
#include <rexx/storage.h>
#include <xpk/xpk.h>
#include <clib/alib_protos.h>
#ifdef __MORPHOS__
#define NO_PPCINLINE_STDARG
#include <ppcinline/locale.h>
#include <ppcinline/socket.h>
#else
#include <proto/socket.h>
#include <proto/locale.h>
#endif
#include <proto/muimaster.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/utility.h>
#include <proto/icon.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/datatypes.h>
#include <proto/wb.h>
#include <proto/iffparse.h>
#include <proto/keymap.h>
#include <proto/rexxsyslib.h>
#include <proto/xpkmaster.h>
#include <proto/openurl.h>
#include <proto/miami.h>
#include <proto/genesis.h>
#include <proto/cmanager.h>
#include <proto/pm.h>
#include <clib/macros.h>
#include <NewReadArgs.h>
#include <extra.h>


#ifdef __MORPHOS__
#undef DoSuperMethod
#define DoSuperMethod(cl,obj,a,b,c) ({ LONG m[] = { (LONG)(a), (LONG)(b), (LONG)(c) }; DoSuperMethodA(cl,obj,(Msg)m); })
#define KPrintF dprintf

/*MorphOS standard netincludes don't have these*/

struct in_addr {
  u_long s_addr;
};

struct sockaddr_in {
  u_char sin_len;
  u_char sin_family;
  u_short sin_port;
  struct in_addr sin_addr;
  char sin_zero[8];
};

 struct hostent {
  char *h_name;
  char **h_aliases;
  int h_addrtype;
  int h_length;
  char **h_addr_list;
  #define h_addr h_addr_list[0]

  #define Shutdown shutdown
  #define GetHostByName gethostbyname
  #define Connect connect
  #define Recv recv
  #define Send send
  #define Socket socket

  #define SMTP_NO_SOCKET -1

  #define SOCK_STREAM 1
  #define AF_INET 2
  #define EINPROGRESS 36

};

#else
#include <clib/locale_protos.h>
#endif

#if (defined DEBUG) || (defined _MGST)
  #include "clib/debug_protos.h"
#endif

#if (defined DEBUG)
  #define DB(x) x
  #define DBpr(x) (KPrintF("YAM: %s",x))
#else
  #define DB(x)
  #define DBpr(x)
#endif

#include "YAM_locale.h"

/// Defines
#if defined __PPC__
	#define CPU " [PPC]"
#elif defined _M68060
	#define CPU " [060]"
#elif defined _M68040
	#define CPU " [040]"
#elif defined _M68020
	#define CPU " [020]"
#else
	#define CPU ""
#endif

/* extern define YAMVER */
#define __YAM_VERSION YAMVER CPU

#define ANYBOX NULL

#define True  ((BOOL)TRUE)
#define False ((BOOL)FALSE)

#define FOCOLNUM 5
#define MACOLNUM 7
#define ABCOLNUM 9


#define AddrName(abentry) ((abentry).RealName[0]?(abentry).RealName:(abentry).Address)
#define FolderName(fo)    ((fo) ? (fo)->Name : "?")
#define OUTGOING(type)    (type == FT_OUTGOING || type == FT_SENT || type == FT_CUSTOMSENT)
#define Virtual(mail)     (((mail)->Flags&MFLAG_NOFOLDER) == MFLAG_NOFOLDER)

#define DBG  printf("File %s, Func %s, Line %d\n",__FILE__,__FUNC__,__LINE__);
#define DBGP DBG Delay(100);
#define clear(p,l) memset((p), 0, (l));


#define MUIA_Slider_Weights          0x80002511
#define MUIA_Dtpic_Name              0x80423d72
#define MUIM_GoActive                0x8042491a
#define MUIM_GoInactive              0x80422c0c

#define MUIX_PC "\0335"

#define PGPLOGFILE "T:PGP.log"
#define NOERRORS   16
#define KEEPLOG    32
#define CRYPTBYTE 164

#define MSG_Space ((APTR)1)
#define OUT_NIL ((BPTR)1)
#define MFLAG_MULTIRCPT   1
#define MFLAG_MULTIPART   2
#define MFLAG_REPORT      4
#define MFLAG_CRYPT       8
#define MFLAG_SIGNED     16
#define MFLAG_SENDERINFO 32
#define MFLAG_SENDMDN    64
#define MFLAG_NOFOLDER  128
#define MDN_TYPEMASK  0x0F
#define MDN_AUTOACT   0x10
#define MDN_AUTOSEND  0x20
#define PGPE_MIME     1
#define PGPE_OLD      2
#define PGPS_MIME     1
#define PGPS_OLD      2
#define PGPS_BADSIG   4
#define PGPS_ADDRESS  8
#define PGPS_CHECKED 16
#define HIDE_INFO    1
#define HIDE_XY      2
#define HIDE_TBAR    4
#define NOTI_REQ     1
#define NOTI_SOUND   2
#define NOTI_CMD     4
#define ATTREQ_DISP  0
#define ATTREQ_SAVE  1
#define ATTREQ_PRINT 2
#define ATTREQ_MULTI 32
#define NEWF_QUIET 1
#define NEWF_REP_NOQUOTE  2
#define NEWF_REP_PRIVATE  4
#define NEWF_REP_MLIST    8
#define NEWF_FWD_NOATTACH 16

#define PRINTMETHOD_DUMPRAW    (0)
#define PRINTMETHOD_LATEX      (1)
#define PRINTMETHOD_POSTSCRIPT (2)	// not yet implemented


struct MimeView
{
   char  ContentType[SIZE_CTYPE];
   char  Command[SIZE_COMMAND];
   char  Extension[SIZE_NAME];
};

struct Rule
{
   char  Name[SIZE_NAME];
   BOOL  Remote;
   BOOL  ApplyToNew;
   BOOL  ApplyOnReq;
   BOOL  ApplyToSent;
   int   Combine;
   int   Field[2];
   int   SubField[2];
   char  CustomField[2][SIZE_DEFAULT];
   int   Comparison[2];
   char  Match[2][SIZE_PATTERN];
   BOOL  CaseSens[2];
   BOOL  Substring[2];
   int   Actions;
   char  BounceTo[SIZE_ADDRESS];
   char  ForwardTo[SIZE_ADDRESS];
   char  ReplyFile[SIZE_PATHFILE];
   char  ExecuteCmd[SIZE_COMMAND];
   char  PlaySound[SIZE_PATHFILE];
   char  MoveTo[SIZE_NAME];
   char  **PatternsFromList;
};       

struct TranslationTable
{
   char Name[SIZE_DEFAULT];
   char File[SIZE_PATHFILE];
   char SourceCharset[SIZE_NAME], DestCharset[SIZE_NAME];
   BOOL Header;
   UBYTE Table[256];
};

struct Column
{
   char *Name;
   int   Index;
   int   DefWidth;
   int   Width;
   int   Position;
   BOOL  Show;
};
///
/// Miscellaneous structures

struct Data2D
{
   int Allocated;
   int Used;
   char **Data;
};

struct MailInfo
{
   int   Pos;
   char *FName;
   BOOL  Display;
};
        
struct ComprMail
{
   int    Flags;
   char   MailFile[SIZE_MFILE];
   struct DateStamp Date;
   char   Status;
   char   Importance;
   long   cMsgID, cIRTMsgID;
   long   Size;
   int    MoreBytes;
};

struct TempFile
{
   char Filename[SIZE_PATHFILE];
   FILE *FP;
};

struct TransStat
{
   int Msgs_Tot, Msgs_Done;
   long Size_Tot, Size_Done, Size_Curr, Delay;
   long Clock_Start, Clock_Last;
};

struct Search
{
   int  Mode, PersMode, Compare, Status, Fast;
   BOOL CaseSens, SubString;
   char Match[SIZE_PATTERN+4], PatBuf[SIZE_PATTERN], *Pattern;
   char Field[SIZE_DEFAULT];
   struct DateTime DT;
   struct Data2D List;
   struct Rule *Rule;
   long Size;
};

struct BodyChunkData
{
   ULONG *Colors;
   UBYTE *Body;
   int Width, Height, Depth;
   int Compression, Masking;
   char File[SIZE_NAME];
};

struct ExpandTextData
{
   char *OS_Name;
   char *OS_Address;
   char *OM_Subject;
   struct DateStamp *OM_Date;
   char *OM_MessageID;
   char *R_Name;
   char *R_Address;
   char *HeaderFile;
};

struct PageList
{
   int Offset;
   APTR PageLabel;
};

struct FIndex
{
   ULONG ID;
   int   Total, New, Unread;
   int   Size;
   long  reserved[2];
};
///
/// Module Classes
struct SearchGroup
{
   APTR PG_SRCHOPT, CY_MODE, ST_FIELD, CY_COMP[5], ST_MATCH[5], BT_FILE[5], BT_EDIT[5];
   APTR RA_ADRMODE, CY_STATUS, CH_CASESENS[5], CH_SUBSTR[5];
};

struct CO_ClassData  /* configuration window */
{
   struct CO_GUIData
   {
      APTR WI, BT_SAVE, BT_USE, BT_CANCEL;
      APTR MI_IMPMIME, LV_PAGE, GR_PAGE, GR_SUBPAGE;
/*0*/      APTR ST_REALNAME, ST_EMAIL, ST_POPHOST0, ST_PASSWD0, CY_TZONE, CH_DLSAVING;
/*1*/      APTR ST_SMTPHOST, ST_DOMAIN, CH_SMTP8BIT, CH_USESMTPAUTH, ST_SMTPAUTHUSER , ST_SMTPAUTHPASS ,LV_POP3, GR_POP3, BT_PADD, BT_PDEL, ST_POPHOST, ST_POPUSERID, ST_PASSWD, CH_DELETE, CH_USEAPOP, CH_POPENABLED;
/*2*/      APTR CH_AVOIDDUP, CY_MSGSELECT, CY_TRANSWIN, CH_UPDSTAT, ST_WARNSIZE, ST_INTERVAL, CH_DLLARGE, CH_NOTIREQ, CH_NOTISOUND, CH_NOTICMD, ST_NOTISOUND, ST_NOTICMD;
/*3*/      APTR LV_RULES, BT_RADD, BT_RDEL, ST_RNAME, CH_REMOTE, CY_COMBINE[2], GR_LRGROUP, GR_LOCAL, GR_REMOTE, PO_MOVETO, TX_MOVETO, LV_MOVETO, CH_APPLYNEW, CH_APPLYREQ, CH_APPLYSENT, CH_ABOUNCE, ST_ABOUNCE, CH_AFORWARD, ST_AFORWARD, CH_ARESPONSE, ST_ARESPONSE, CH_AEXECUTE, ST_AEXECUTE, CH_APLAY, ST_APLAY, CH_AMOVE, CH_ADELETE, CH_ASKIP;
           struct SearchGroup GR_SEARCH[4];
/*4*/      APTR CY_HEADER, ST_HEADERS, CY_SENDERINFO, CY_SIGSEPLINE, CA_COLTEXT, CA_COL2QUOT, CH_FIXFEDIT, CH_ALLTEXTS, CH_MULTIWIN, CH_WRAPHEAD, CH_TEXTSTYLES, ST_INTRANS;
/*5*/      APTR ST_REPLYTO, ST_ORGAN, ST_EXTHEADER, ST_HELLOTEXT, ST_BYETEXT, ST_OUTTRANS, ST_EDWRAP, CY_EDWRAP, ST_EDITOR, CH_LAUNCH;
/*6*/      APTR ST_REPLYHI, ST_REPLYTEXT, ST_REPLYBYE, ST_AREPLYHI, ST_AREPLYTEXT, ST_AREPLYBYE, ST_AREPLYPAT, ST_MREPLYHI, ST_MREPLYTEXT, ST_MREPLYBYE, CH_QUOTE, ST_REPLYCHAR, ST_ALTQUOTECHAR, CH_QUOTEEMPTY, CH_COMPADDR, CH_STRIPSIG, ST_FWDSTART, ST_FWDEND;
/*7*/      APTR CH_USESIG, CY_SIGNAT, BT_SIGEDIT, TE_SIGEDIT, BT_INSTAG, BT_INSENV, ST_TAGFILE, ST_TAGSEP;
/*8*/      APTR CH_FCOLS[FOCOLNUM], CH_MCOLS[MACOLNUM], CH_FIXFLIST, CH_BEAT;
/*9*/     APTR ST_PGPCMD, ST_MYPGPID, CH_ENCSELF, ST_REMAILER, ST_FIRSTLINE, ST_LOGFILE, CY_LOGMODE, CH_SPLITLOG, CH_LOGALL;
/*10*/     APTR CH_POPSTART, CH_SENDSTART, CH_DELETESTART, CH_REMOVESTART, CH_LOADALL, CH_MARKNEW, CH_CHECKBD, CH_SENDQUIT, CH_DELETEQUIT, CH_REMOVEQUIT;
/*11*/     APTR LV_MIME, GR_MIME, ST_CTYPE, ST_EXTENS, ST_COMMAND, ST_DEFVIEWER, BT_MADD, BT_MDEL, CH_IDENTBIN, ST_DETACHDIR, ST_ATTACHDIR;
/*12*/     APTR ST_GALLDIR, ST_PROXY, ST_PHOTOURL, CH_ADDINFO, CY_ATAB, ST_NEWGROUP, CH_ACOLS[ABCOLNUM];
/*13*/     APTR LV_REXX, ST_RXNAME, ST_SCRIPT, CY_ISADOS, CH_CONSOLE, CH_WAITTERM;
/*14*/     APTR ST_TEMPDIR, ST_APPX, ST_APPY, CH_CLGADGET, CH_CONFIRM, NB_CONFIRMDEL, CH_REMOVE, CH_SAVESENT;
           APTR RA_MDN_DISP, RA_MDN_PROC, RA_MDN_DELE, RA_MDN_RULE, CH_SEND_MDN, TX_PACKER, TX_ENCPACK, NB_PACKER, NB_ENCPACK, ST_ARCHIVER;
   } GUI;
   int VisiblePage;
   int LastSig;
   BOOL Visited[MAXCPAGES];
   BOOL UpdateAll;
};

struct FI_ClassData  /* find window */
{
   struct FI_GUIData
   {
      APTR WI;
      APTR LV_FOLDERS;
      struct SearchGroup GR_SEARCH;
      APTR LV_MAILS, GR_PAGE, GA_PROGRESS, BT_SELECT, BT_READ;
   } GUI;
  long Abort;
};

struct UniversalClassData
{
   struct UniversalGUIData { APTR WI; } GUI;
};
///
/// MUI internal custom classes data
struct DumData { long dummy; };

struct CW_Data
{
   char Buf[32];
   BOOL Weights;
};

struct BC_Data
{
   struct BodyChunkData *BCD;
};

struct AL_Data 
{ 
   struct Hook DisplayHook;
   Object *Object;
   APTR Image;
};
struct PL_Data
{ 
   struct Hook DisplayHook;
   Object *Object[MAXCPAGES];
   APTR Image[MAXCPAGES];
};
struct WS_Data
{
   struct MUI_EventHandlerNode ehnode;
};
///
/// Enumerations
enum { STATUS_UNR, STATUS_OLD, STATUS_FWD, STATUS_RPD, STATUS_WFS, STATUS_ERR, STATUS_HLD, STATUS_SNT, STATUS_NEW, STATUS_DEL, STATUS_LOA, STATUS_SKI };

enum { CT_TX_PLAIN=0, CT_TX_HTML, CT_TX_GUIDE,
       CT_AP_OCTET, CT_AP_PS, CT_AP_RTF, CT_AP_LHA, CT_AP_LZX, CT_AP_ZIP, CT_AP_AEXE, CT_AP_SCRIPT, CT_AP_REXX,
       CT_IM_JPG, CT_IM_GIF, CT_IM_PNG, CT_IM_TIFF, CT_IM_ILBM,
       CT_AU_AU, CT_AU_8SVX, CT_AU_WAV,
       CT_VI_MPG, CT_VI_MOV, CT_VI_ANIM, CT_VI_AVI,
       CT_ME_EMAIL };

enum { PA_LOAD, PA_DELETE, PA_SKIP, 
       PM_ALL, PM_TEXTS, PM_NONE,
       ABM_EDIT, ABM_TO, ABM_CC, ABM_BCC, ABM_REPLYTO, ABM_FROM,
       ED_OPEN, ED_INSERT, ED_INSQUOT, ED_INSALTQUOT, ED_INSROT13, ED_PASQUOT, ED_PASALTQUOT, ED_PASROT13,
};


enum { SEND_ALL = -2, SEND_ACTIVE, NEW_NEW, NEW_REPLY, NEW_FORWARD, NEW_BOUNCE, NEW_EDIT, NEW_SAVEDEC,
       POP_USER, POP_START, POP_TIMED, POP_REXX, APPLY_USER, APPLY_AUTO, APPLY_SENT, APPLY_REMOTE,
       APPLY_RX_ALL, APPLY_RX, WRITE_HOLD, WRITE_SEND, WRITE_QUEUE, RIM_QUIET, RIM_READ, RIM_EDIT,
       RIM_QUOTE, RIM_PRINT, RCPT_TYPE_ALL, RCPT_TYPE_READ,
       ABF_USER, ABF_RX, ABF_RX_NAME, ABF_RX_EMAIL, ABF_RX_NAMEEMAIL,
       };

enum { FS_NONE=0, FS_FROM, FS_TO, FS_CC, FS_REPLYTO, FS_SUBJECT, FS_DATE, FS_SIZE };

enum { MDN_IGNORE=0, MDN_DENY, MDN_READ, MDN_DISP, MDN_PROC, MDN_DELE };

enum { MACRO_MEN0=0, MACRO_MEN1, MACRO_MEN2, MACRO_MEN3, MACRO_MEN4, MACRO_MEN5,
       MACRO_MEN6, MACRO_MEN7, MACRO_MEN8, MACRO_MEN9, MACRO_STARTUP, MACRO_QUIT,
       MACRO_PREGET, MACRO_POSTGET, MACRO_NEWMSG, MACRO_PRESEND, MACRO_POSTSEND,
       MACRO_READ, MACRO_PREWRITE, MACRO_POSTWRITE, MACRO_URL };

///
/// Declaration of external variables
extern struct Config *CE;
extern struct Hook AB_FromAddrBookHook;
extern struct Hook MA_ChangeSelectedHook, MA_ChangeFolderHook, MA_SendHook, MA_RescanIndexHook, MA_FlushIndexHook, MA_ApplyRulesHook, MA_DeleteDeletedHook, MA_DeleteOldHook;
extern struct Hook MA_LV_Cmp2Hook, MA_LV_FCmp2Hook, MA_LV_DspFuncHook, MA_LV_FDspFuncHook;
extern struct Hook PO_InitFolderListHook, MA_PO_MoveHook, PO_WindowHook;
extern struct Hook CO_OpenHook, CO_RemoteToggleHook;
extern struct Hook CO_EditSignatHook, CO_ToggleColHook, CO_GetDefaultPOPHook, CO_GetP3EntryHook, CO_PutP3EntryHook, CO_AddPOP3Hook, CO_DelPOP3Hook;
extern struct Hook CO_GetFOEntryHook, CO_PutFOEntryHook, CO_AddFolderHook, CO_DelFolderHook, CO_GetRUEntryHook, CO_PutRUEntryHook, CO_AddRuleHook, CO_DelRuleHook;
extern struct Hook CO_GetMVEntryHook, CO_PutMVEntryHook, CO_AddMimeViewHook, CO_DelMimeViewHook, CO_GetRXEntryHook, CO_PutRXEntryHook;
extern struct Hook FI_OpenHook;
extern struct Hook DI_OpenHook;
extern struct Hook US_OpenHook;
extern struct Hook AB_OpenHook;
extern struct Hook RE_PrevNextHook, RE_LV_AttachDspFuncHook, RE_CloseHook;
extern struct Hook WR_NewMailHook, WR_EditHook;
extern struct Hook GeneralDesHook, RestartHook,DisposeModuleHook;
extern char *Status[9];
extern char *ContType[MAXCTYPE+1];
extern APTR ContTypeDesc[MAXCTYPE];
extern char *SigNames[3], *FolderNames[4], *SecCodes[5];
extern char *months[12], *wdays[7];
extern struct MUI_CustomClass *CL_BodyChunk, *CL_FolderList, *CL_MailList, *CL_AddressList, *CL_TextEditor, *CL_PageList;
extern struct MUI_CustomClass *CL_DDList, *CL_AttachList, *CL_DDString, *CL_MainWin;
extern struct Data2D Header;
extern int Mode2Group[12];
extern int BusyLevel;
///
/// Function prototypes
extern char *Protection2(void);
extern int StringRequest(char *, int, char *, char *, char *, char *, char *, BOOL, APTR);
extern struct Folder *FolderRequest(char *, char *, char *, char *, struct Folder *, APTR);
extern struct Part *AttachRequest(char *, char *, char *, char *, int, int, APTR);
extern void InfoWindow(char *, char *, char *, APTR);
extern char *GetLine(FILE *, char *, int);
extern BOOL CopyFile(char *, FILE *, char *, FILE *);
extern BOOL RenameFile(char *, char *);
extern BOOL ConvertCRLF(char *, char *, BOOL);
extern void QuoteWordWrap(char *, int, char *, char *, FILE *);
extern void SimpleWordWrap(char *, int);
extern int FileSize(char *);
extern long FileProtection(char *);
extern int FileType(char *);
extern BOOL FileExists(char *);
extern BOOL PFExists(char *, char *);
extern int ReqFile(int, Object *, char *, int, char *, char *);
extern struct TempFile *OpenTempFile(char *);
extern void CloseTempFile(struct TempFile *);
extern BOOL DumpClipboard(FILE *);
extern void DeleteMailDir(char *, BOOL);
extern BOOL CheckPrinter(void);
extern int MatchNoCase(char *, char *);
extern BOOL MatchTT(char *, struct TranslationTable *, BOOL);
extern BOOL ISpace(char);
extern BOOL isSpace(int), isGraph(int), isAlNum(int);
extern char *StripUnderscore(char *);
extern char *GetNextLine(char *);
extern char *TrimStart(char *);
extern char *TrimEnd(char *);
extern char *Trim(char *);
extern char *itoa(int);
extern char *stccat(char *, char *, int);
extern char *stristr(char *, char *);
extern char *MyStrChr(char *, int);
extern char *AllocStrBuf(long);
extern void FreeStrBuf(char *);
extern char *StrBufCpy(char *, char *);
extern char *StrBufCat(char *, char *);
extern void FreeData2D(struct Data2D *);
extern char *AllocData2D(struct Data2D *, int);
extern APTR AllocCopy(APTR, int);
extern char *Encrypt(char *);
extern char *Decrypt(char *);
extern char *DescribeCT(char *);
extern char *BuildAddrName(char *,char *);
extern char *CreateFilename(char *);
extern BOOL CreateDirectory(char *);
extern char *GetFolderDir(struct Folder *);
extern char *GetMailFile(char *, struct Folder *, struct Mail *);
extern time_t GetDateStamp(void);
extern char *DateStamp2String(struct DateStamp *, int);
extern long DateStamp2Long(struct DateStamp *);
extern char *GetTZ(void);
extern struct DateStamp *ScanDate(char *);
extern void FormatSize(int, char *);
extern void MyAddTail(struct Mail **, struct Mail *);
extern void MyRemove(struct Mail **, struct Mail *);
extern APTR WhichLV(struct Folder *);
extern struct MailInfo *GetMailInfo(struct Mail *);
extern char *ExpandText(char *, struct ExpandTextData *);
extern BOOL TransferMailFile(BOOL, struct Mail *, struct Folder *);
extern BOOL MailExists(struct Mail *, struct Folder *);
extern int SelectMessage(struct Mail *);
extern void DisplayMailList(struct Folder *, APTR);
extern struct Mail *AddMailToList(struct Mail *, struct Folder *);
extern void RemoveMailFromList(struct Mail *);
extern void ClearMailList(struct Folder *, BOOL);
extern struct Person *GetReturnAddress(struct Mail *);
extern char ShortCut(char *);
extern void SetHelp(APTR, APTR);
extern void DisposeModulePush(void *);
extern void DisposeModule(void *);
extern Object *MakeButton(char *);
extern Object *MakeCycle(char **, char *);
extern Object *MakeImage(UBYTE *);
extern void FreeBCImage(struct BodyChunkData *);
extern struct BodyChunkData *GetBCImage(char *);
extern struct BodyChunkData *LoadBCImage(char *);
extern Object *MakeCheck(char *);
extern Object *MakeCheckGroup(Object **, char *);
extern Object *MakeString(int, char *);
extern Object *MakePassString(char *);
extern Object *MakeInteger(int, char *);
extern Object *MakePGPKeyList(APTR *, BOOL, char *);
extern Object *MakePicture(char *);
extern Object *MakeStatusFlag(char *);
extern Object *MakeNumeric(int, int, BOOL);
extern Object *MakeMenuitem(const UBYTE *str, ULONG ud);
extern void SetupToolbar(struct MUIP_Toolbar_Description *, char *, char *, UWORD);
extern void SetupMenu(int, struct NewMenu *, char *, char *, int);
extern int GetMUI(Object *,int);
extern char *GetMUIStringPtr(Object *);
extern void GetMUIString(char *,Object *);
extern void GetMUIText(char *,Object *);
extern int GetMUIInteger(Object *);
extern int GetMUINumer(Object *);
extern BOOL GetMUICheck(Object *);
extern int GetMUICycle(Object *);
extern int GetMUIRadio(Object *);
extern BOOL SafeOpenWindow(Object *);
extern ULONG DoSuperNew(struct IClass *, Object *, ULONG,...);
extern void SaveLayout(BOOL);
extern void LoadLayout(void);
extern ULONG ConvertKey(struct IntuiMessage *);
extern void DisplayStatistics(struct Folder *);
extern void SetupAppIcons(void);
extern BOOL EditorToFile(Object *, char *, struct TranslationTable *);
extern BOOL FileToEditor(char *, Object *);
extern BOOL LoadTranslationTable(struct TranslationTable **, char *);
extern int CompressMsgID(char *);
extern BOOL RepackMailFile(struct Mail *, int, char *);
extern char *StartUnpack(char *, char *, struct Folder *);
extern void FinishUnpack(char *);
extern BOOL DoPack(char *, char *, struct Folder *);
extern BOOL IsValidMailFile(char *);
extern void Busy(char *, char *, int, int);
extern void PlaySound(char *);
extern BOOL MatchExtension(char *, char *);
extern char *IdentifyFile(char *);
extern void InsertAddresses(APTR, char **, BOOL);
extern char *AllocReqText(char *);
extern void SPrintF(char *outstr, char *fmtstr, ...);
extern void PopUp(void);
extern void AppendLog(int, char *, void *, void *, void *, void *);
extern void AppendLogNormal(int, char *, void *, void *, void *, void *);
extern void AppendLogVerbose(int, char *, void *, void *, void *, void *);
extern int PGPCommand(char *, char *, int);
extern void PGPGetPassPhrase(void);
extern void PGPClearPassPhrase(BOOL);
extern BOOL ExecuteCommand(char *, BOOL, BPTR);
extern int GetSimpleID(void);
extern void GotoURL(char *);
extern char *strtok_r(char**, char*);

extern void MA_SetSortFlag(void);
extern BOOL MA_PromptFolderPassword(struct Folder *, APTR);
extern void MA_DeleteSingle(struct Mail *, BOOL);
extern BOOL MA_ReadHeader(FILE *);
extern BOOL MA_SaveIndex(struct Folder *);
extern int  MA_LoadIndex(struct Folder *, BOOL);
extern BOOL MA_GetIndex(struct Folder *);
extern void MA_UpdateIndexes(BOOL);
extern void MA_ExpireIndex(struct Folder *);
extern void MA_FlushIndexes(BOOL);
extern char *MA_NewMailFile(struct Folder *, char *, int);
extern struct Mail *MA_GetActiveMail(struct Folder *, struct Folder **, int *);
extern void MA_About(void);
extern struct ExtendedMail *MA_ExamineMail(struct Folder *, char *, char *, BOOL);
extern void MA_FreeEMailStruct(struct ExtendedMail *);
extern void MA_ScanMailBox(struct Folder *);
extern void MA_ChangeFolder(struct Folder *);
extern ULONG MA_FolderContextMenu(struct MUIP_ContextMenuBuild *msg);
extern struct Mail **MA_CreateMarkedList(APTR);
extern void MA_ChangeTransfer(BOOL);
extern int MA_NewNew(struct Mail *, int);
extern int MA_NewEdit(struct Mail *, int, int);
extern int MA_NewBounce(struct Mail *, int);
extern int MA_NewForward(struct Mail **, int);
extern int MA_NewReply(struct Mail **, int);
extern int MA_NewMessage(int, int);
extern void MA_MoveCopy(struct Mail *, struct Folder *, struct Folder *, BOOL);
extern ULONG MA_MailListContextMenu(struct MUIP_ContextMenuBuild *msg);
extern void MA_GetAddress(struct Mail **);
extern void MA_ChangeSubject(struct Mail *, char *);
extern void MA_RemoveAttach(struct Mail *);
extern void MA_SetMailStatus(struct Mail *, int);
extern void MA_SetStatusTo(int);
extern BOOL MA_ImportMessages(char *);
extern BOOL MA_ExportMessages(BOOL, char *, BOOL);
extern void MA_DeleteMessage(BOOL delatonce, BOOL force);
extern BOOL MA_StartMacro(int, char *);
extern int MA_AllocRules(struct Search **, int);
extern void MA_FreeRules(struct Search **, int);
extern BOOL MA_ExecuteRuleAction(struct Rule *, struct Mail *);
extern BOOL MA_SendMList(struct Mail **);
extern BOOL MA_Send(int);
extern void MA_PopNow(int, int);
extern void MA_SetupDynamicMenus(void);
extern int MA_CmpFunc(struct Mail **, struct Mail **);
extern void MA_MakeFOFormat(APTR);
extern void MA_MakeMAFormat(APTR);
extern struct MA_ClassData *MA_New(void);

extern BOOL CO_IsValid(void);
extern struct Rule *CO_NewRule(void);
extern struct POP3 *CO_NewPOP3(struct Config *, BOOL);
extern struct MimeView *CO_NewMimeView(void);
extern void CO_SetDefaults(struct Config *, int);
extern struct Rule *CO_NewRule(void);
extern void CO_RuleGhost(struct CO_GUIData *, struct Rule *);
extern BOOL CO_LoadConfig(struct Config *, char *, struct Folder ***);
extern void CO_SaveConfig(struct Config *, char *);
extern void CO_Validate(struct Config *, BOOL);
extern void CO_SetConfig(void);
extern void CO_GetConfig(void);
extern void CO_FreeConfig(struct Config *);

extern APTR AB_GotoEntry(char *alias);
extern void AB_InsertAddress(APTR, char *, char *, char *);
extern char *AB_CompleteAlias(char *);
extern char *AB_ExpandBD(long date);
extern long AB_CompressBD(char *datestr);
extern void AB_CheckBirthdates(void);
extern BOOL AB_LoadTree(char *, BOOL, BOOL);
extern BOOL AB_SaveTree(char *);
extern void AB_MakeABFormat(APTR);
extern struct AB_ClassData *AB_New(void);

extern BOOL RE_DoMDN(int MDNtype, struct Mail *mail, BOOL multi);
extern struct Mail *RE_GetQuestion(long);
extern struct Mail *RE_GetAnswer(long);
extern BOOL RE_DecodePart(struct Part *);
extern void RE_CleanupMessage(int);
extern BOOL RE_LoadMessage(int, int);
extern char *RE_ReadInMessage(int, int);
extern void RE_SaveAll(int, char *);
extern char *RE_SuggestName(struct Mail *);
extern BOOL RE_Export(int, char *, char *, char *, int, BOOL, BOOL, char *);
extern void RE_SaveDisplay(int, FILE *);
extern void RE_DisplayMIME(char *, char *);
extern void RE_DisplayMessage(int);
extern void RE_InitPrivateRC(struct Mail *, int);
extern void RE_FreePrivateRC(void);
extern void RE_ReadMessage(int, struct Mail *);
extern int RE_Open(int, BOOL);
extern struct RE_ClassData *RE_New(int, BOOL);

extern int WR_ResolveName(int, char *, char **, BOOL);
extern char *WR_ExpandAddresses(int, char *, BOOL, BOOL);
extern void WR_AddSignature(char *, int);
extern void WR_OpenWin(int);
extern void WR_FromAddrBook(Object *);
extern void EmitHeader(FILE *, char *, char *);
extern struct WritePart *NewPart(int);
extern int WR_Open(int, BOOL);
extern void WR_SetupOldMail(int);
extern void FreePartsList(struct WritePart *);
extern BOOL WR_AddFileToList(int, char *, char *, BOOL);
extern char *WR_AutoSaveFile(int);
extern void WR_NewMail(int, int);
extern void WR_Cleanup(int);
extern void WR_App(int, struct AppMessage *);
extern struct WR_ClassData *WR_New(int);

extern void ER_NewError(char *, char *, char *);

extern void FI_SearchGhost(struct SearchGroup *, BOOL disabled);
extern APTR FI_ConstructSearchGroup(struct SearchGroup *, BOOL);
extern BOOL MyMatch(BOOL, char *, char *);
extern BOOL FI_PrepareSearch(struct Search *, int, BOOL, int, int, int, BOOL, char *, char *);
extern BOOL FI_DoComplexSearch(struct Search *, int, struct Search *, struct Mail *);

extern BOOL US_Login(char *, char *, char *, char *);
extern struct User *US_GetCurrentUser(void);

extern void NewLine(FILE *, BOOL);
extern void to64(FILE *, FILE *, BOOL);
extern void toqp(FILE *, FILE *);
extern void touue(FILE *, FILE *);
extern void fromform(FILE *, FILE *, struct TranslationTable *);
extern void fromqp(FILE *, FILE *, struct TranslationTable *);
extern void from64(FILE *, FILE *, struct TranslationTable *, BOOL);
extern void fromuue(FILE *, FILE *);
extern void fromqptxt(char *, char *, struct TranslationTable *);
extern void from64txt(char *, char *, struct TranslationTable *);
extern void fromuuetxt(char **, FILE *);
extern BOOL DoesNeedPortableNewlines(char *);

extern BOOL InitClasses(void);
extern void ExitClasses(void);
