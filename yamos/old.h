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

#include <workbench/workbench.h>
#include <workbench/startup.h>
#include <devices/printer.h>
#include <intuition/icclass.h>
#include <intuition/gadgetclass.h>
#include <datatypes/pictureclass.h>
#include <datatypes/soundclass.h>
#include <libraries/asl.h>
#include <libraries/gadtools.h>
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
#include <clib/alib_protos.h>
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
#define PRINTMETHOD_POSTSCRIPT (2)  // not yet implemented


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
/*8*/      APTR CH_FCOLS[FOCOLNUM], CH_MCOLS[MACOLNUM], CH_FIXFLIST, CH_BEAT, CY_SIZE;
/*9*/     APTR ST_PGPCMD, ST_MYPGPID, CH_ENCSELF, ST_REMAILER, ST_FIRSTLINE, ST_LOGFILE, CY_LOGMODE, CH_SPLITLOG, CH_LOGALL;
/*10*/     APTR CH_POPSTART, CH_SENDSTART, CH_DELETESTART, CH_REMOVESTART, CH_LOADALL, CH_MARKNEW, CH_CHECKBD, CH_SENDQUIT, CH_DELETEQUIT, CH_REMOVEQUIT;
/*11*/     APTR LV_MIME, GR_MIME, ST_CTYPE, ST_EXTENS, ST_COMMAND, ST_DEFVIEWER, BT_MADD, BT_MDEL, CH_IDENTBIN, ST_DETACHDIR, ST_ATTACHDIR;
/*12*/     APTR ST_GALLDIR, ST_PROXY, ST_PHOTOURL, CH_ADDINFO, CY_ATAB, ST_NEWGROUP, CH_ACOLS[ABCOLNUM];
/*13*/     APTR LV_REXX, ST_RXNAME, ST_SCRIPT, CY_ISADOS, CH_CONSOLE, CH_WAITTERM;
/*14*/     APTR ST_TEMPDIR, ST_APPX, ST_APPY, CH_CLGADGET, CH_CONFIRM, NB_CONFIRMDEL, CH_REMOVE, CH_SAVESENT;
           APTR RA_MDN_DISP, RA_MDN_PROC, RA_MDN_DELE, RA_MDN_RULE, CH_SEND_MDN, TX_PACKER, TX_ENCPACK, NB_PACKER, NB_ENCPACK, ST_ARCHIVER, ST_APPICON;
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

extern BOOL RE_DoMDN(int MDNtype, struct Mail *mail, BOOL multi);
extern struct Mail *RE_GetQuestion(long);
extern struct Mail *RE_GetAnswer(long);
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

