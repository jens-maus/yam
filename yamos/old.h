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
#endif

#define ANYBOX NULL

#define AddrName(abentry) ((abentry).RealName[0]?(abentry).RealName:(abentry).Address)
#define FolderName(fo)    ((fo) ? (fo)->Name : "?")
#define OUTGOING(type)    (type == FT_OUTGOING || type == FT_SENT || type == FT_CUSTOMSENT)
#define Virtual(mail)     (((mail)->Flags&MFLAG_NOFOLDER) == MFLAG_NOFOLDER)

#define MUIA_Slider_Weights          0x80002511
#define MUIA_Dtpic_Name              0x80423d72
#define MUIM_GoActive                0x8042491a
#define MUIM_GoInactive              0x80422c0c

#define MUIX_PC "\0335"

#define PGPLOGFILE "T:PGP.log"
#define NOERRORS   16
#define KEEPLOG    32
#define CRYPTBYTE 164

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

struct TransStat
{
   int Msgs_Tot, Msgs_Done;
   long Size_Tot, Size_Done, Size_Curr, Delay;
   long Clock_Start, Clock_Last;
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

struct UniversalClassData
{
   struct UniversalGUIData { APTR WI; } GUI;
};

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

enum { CT_TX_PLAIN=0, CT_TX_HTML, CT_TX_GUIDE,
       CT_AP_OCTET, CT_AP_PS, CT_AP_RTF, CT_AP_LHA, CT_AP_LZX, CT_AP_ZIP, CT_AP_AEXE, CT_AP_SCRIPT, CT_AP_REXX,
       CT_IM_JPG, CT_IM_GIF, CT_IM_PNG, CT_IM_TIFF, CT_IM_ILBM,
       CT_AU_AU, CT_AU_8SVX, CT_AU_WAV,
       CT_VI_MPG, CT_VI_MOV, CT_VI_ANIM, CT_VI_AVI,
       CT_ME_EMAIL };

enum { PA_LOAD, PA_DELETE, PA_SKIP, 
       PM_ALL, PM_TEXTS, PM_NONE,
       ED_OPEN, ED_INSERT, ED_INSQUOT, ED_INSALTQUOT, ED_INSROT13, ED_PASQUOT, ED_PASALTQUOT, ED_PASROT13,
};


enum {NEW_NEW, NEW_REPLY, NEW_FORWARD, NEW_BOUNCE, NEW_EDIT, NEW_SAVEDEC,
       APPLY_USER, APPLY_AUTO, APPLY_SENT, APPLY_REMOTE,
       APPLY_RX_ALL, APPLY_RX, WRITE_HOLD, WRITE_SEND, WRITE_QUEUE, RIM_QUIET, RIM_READ, RIM_EDIT,
       RIM_QUOTE, RIM_PRINT, RCPT_TYPE_ALL, RCPT_TYPE_READ,
       };

enum { FS_NONE=0, FS_FROM, FS_TO, FS_CC, FS_REPLYTO, FS_SUBJECT, FS_DATE, FS_SIZE };

enum { MDN_IGNORE=0, MDN_DENY, MDN_READ, MDN_DISP, MDN_PROC, MDN_DELE };

