#ifndef LIBRARIES_OPENURL_H
#define LIBRARIES_OPENURL_H

/*
** openurl.library - universal URL display and browser launcher library
** Written by Troels Walsted Hansen <troels@thule.no>
** Placed in the public domain.
**
** Library definitions and structures.
*/

/**************************************************************************/

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#ifndef EXEC_LISTS_H
#include <exec/lists.h>
#endif

#ifndef EXEC_NODES_H
#include <exec/nodes.h>
#endif

#ifndef UTILITY_TAGITEM_H
#include <utility/tagitem.h>
#endif

/**************************************************************************/

#define URL_Tagbase TAG_USER

/* these tags are all you need to use openurl.library, the rest of the stuff 
   in this include file is only of interest to prefs program writers.
*/

#define URL_Show         (URL_Tagbase + 1) /* BOOL - show/uniconify browser */
#define URL_BringToFront (URL_Tagbase + 2) /* BOOL - bring browser to front */
#define URL_NewWindow    (URL_Tagbase + 3) /* BOOL - open URL in new window */
#define URL_Launch       (URL_Tagbase + 4) /* BOOL - launch browser when not running */

/* this is Version 3 of this structure */

#define UPF_ISDEFAULTS  (1<<0) /* V2 - structure contains the default settings */
#define UPF_PREPENDHTTP (1<<1) /* V3 - prepend "http://" to URLs w/o scheme */
#define UPF_DOMAILTO    (1<<2) /* V3 - mailto URLs get special treatment */

struct URL_Prefs
{
	UBYTE          up_Version;         /* always check this version number! */
	struct MinList up_BrowserList;     /* list of struct URL_BrowserNodes */
	struct MinList up_MailerList;      /* V3 - list of struct URL_MailerNodes */

	LONGBITS       up_Flags;           /* V2 - flags, see above            */
	BOOL           up_DefShow;         /* these BOOLs are the defaults for */
	BOOL           up_DefBringToFront; /* the similarly named tags         */
	BOOL           up_DefNewWindow;    /* they are all new with Version 2  */
	BOOL           up_DefLaunch;
};

#define REXX_CMD_LEN 64

#define UBNF_URLONCMDLINE (1<<0) /* if set, browser supports getting an URL on 
                                    the commandline when launched. obsolete as
                                    of V3 - use %u on commandline instead */
#define UBN_NAME_LEN         32
#define UBN_PATH_LEN        256
#define UBN_PORT_LEN         32
#define UBN_SHOWCMD_LEN      REXX_CMD_LEN
#define UBN_TOFRONTCMD_LEN   REXX_CMD_LEN
#define UBN_OPENURLCMD_LEN   REXX_CMD_LEN
#define UBN_OPENURLWCMD_LEN  REXX_CMD_LEN

struct URL_BrowserNode
{
	struct MinNode ubn_Node;
	LONGBITS       ubn_Flags;                            /* flags, see above */
	TEXT           ubn_Name[UBN_NAME_LEN];               /* name of webbrowser */
	TEXT           ubn_Path[UBN_PATH_LEN];               /* complete path to browser */
	TEXT           ubn_Port[UBN_PORT_LEN];               /* webbrowser arexx port */
	TEXT           ubn_ShowCmd[UBN_SHOWCMD_LEN];         /* command to show/uniconify browser */
	TEXT           ubn_ToFrontCmd[UBN_TOFRONTCMD_LEN];   /* command to bring browser to front */
	TEXT           ubn_OpenURLCmd[UBN_OPENURLCMD_LEN];   /* command to open url */
	TEXT           ubn_OpenURLWCmd[UBN_OPENURLWCMD_LEN]; /* command to open url in new window */
};

#define UMN_NAME_LEN         32
#define UMN_PATH_LEN        256
#define UMN_PORT_LEN         32
#define UMN_SHOWCMD_LEN      REXX_CMD_LEN
#define UMN_TOFRONTCMD_LEN   REXX_CMD_LEN
#define UMN_WRITEMAILCMD_LEN (REXX_CMD_LEN * 2)

struct URL_MailerNode
{
	struct MinNode umn_Node;
	LONGBITS       umn_Flags;                              /* flags, see above */
	TEXT           umn_Name[UMN_NAME_LEN];                 /* name of mailer */
	TEXT           umn_Path[UMN_PATH_LEN];                 /* complete path to mailer */
	TEXT           umn_Port[UMN_PORT_LEN];                 /* mailer arexx port */
	TEXT           umn_ShowCmd[UMN_SHOWCMD_LEN];           /* command to show/uniconify mailer */
	TEXT           umn_ToFrontCmd[UMN_TOFRONTCMD_LEN];     /* command to bring mailer to front */
	TEXT           umn_WriteMailCmd[UMN_WRITEMAILCMD_LEN]; /* command to write mail */
};

#endif  /* LIBRARIES_OPENURL_H */
