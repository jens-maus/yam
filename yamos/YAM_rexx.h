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
/*
 * Source generated with ARexxBox 1.12 (May 18 1993)
 * which is Copyright (c) 1992,1993 Michael Balzer
 */

#ifndef _YAM_rexx_H
#define _YAM_rexx_H

#define RXIF_INIT   1
#define RXIF_ACTION 2
#define RXIF_FREE   3

#define ARB_CF_ENABLED     (1L << 0)

#define ARB_HF_CMDSHELL    (1L << 0)
#define ARB_HF_USRMSGPORT  (1L << 1)

struct RexxHost
{
	struct MsgPort *port;
	char portname[ 80 ];
	long replies;
	struct RDArgs *rdargs;
	long flags;
	APTR userdata;
};

struct rxs_command
{
	char *command, *args, *results;
	long resindex;
	void (*function)( struct RexxHost *, void **, long, struct RexxMsg * );
	long flags;
};

struct arb_p_link
{
	char	*str;
	int		dst;
};

struct arb_p_state
{
	int		cmd;
	struct arb_p_link *pa;
};

#ifndef NO_GLOBALS
extern char RexxPortBaseName[80];
extern struct rxs_command rxs_commandlist[];
extern struct arb_p_state arb_p_state[];
extern int command_cnt;
extern char *rexx_extension;
#endif

void ReplyRexxCommand( struct RexxMsg *rxmsg, long prim, long sec, char *res );
void FreeRexxCommand( struct RexxMsg *rxmsg );
struct RexxMsg *CreateRexxCommand( struct RexxHost *host, char *buff, BPTR fh );
struct RexxMsg *CommandToRexx( struct RexxHost *host, struct RexxMsg *rexx_command_message );
struct RexxMsg *SendRexxCommand( struct RexxHost *host, char *buff, BPTR fh );

void CloseDownARexxHost( struct RexxHost *host );
struct RexxHost *SetupARexxHost( char *basename, struct MsgPort *usrport );
struct rxs_command *FindRXCommand( char *com );
char *ExpandRXCommand( struct RexxHost *host, char *command );
char *StrDup( char *s );
void ARexxDispatch( struct RexxHost *host );

/* rxd-Strukturen dürfen nur AM ENDE um lokale Variablen erweitert werden! */

struct rxd_addrdelete
{
	long rc, rc2;
	struct {
		char *alias;
	} arg;
};

void rx_addrdelete( struct RexxHost *, struct rxd_addrdelete **, long, struct RexxMsg * );

struct rxd_addredit
{
	long rc, rc2;
	struct {
		char *alias;
		char *name;
		char *email;
		char *pgp;
		char *homepage;
		char *street;
		char *city;
		char *country;
		char *phone;
		char *comment;
		long *birthdate;
		char *image;
		char **member;
		long add;
	} arg;
};

void rx_addredit( struct RexxHost *, struct rxd_addredit **, long, struct RexxMsg * );

struct rxd_addrfind
{
	long rc, rc2;
	struct {
		char *var, *stem;
		char *pattern;
		long nameonly;
		long emailonly;
	} arg;
	struct {
		char **alias;
	} res;
};

void rx_addrfind( struct RexxHost *, struct rxd_addrfind **, long, struct RexxMsg * );

struct rxd_addrgoto
{
	long rc, rc2;
	struct {
		char *alias;
	} arg;
};

void rx_addrgoto( struct RexxHost *, struct rxd_addrgoto **, long, struct RexxMsg * );

struct rxd_addrinfo
{
	long rc, rc2;
	struct {
		char *var, *stem;
		char *alias;
	} arg;
	struct {
		char *type;
		char *name;
		char *email;
		char *pgp;
		char *homepage;
		char *street;
		char *city;
		char *country;
		char *phone;
		char *comment;
		long *birthdate;
		char *image;
		char **members;
	} res;
};

void rx_addrinfo( struct RexxHost *, struct rxd_addrinfo **, long, struct RexxMsg * );

struct rxd_addrload
{
	long rc, rc2;
	struct {
		char *filename;
	} arg;
};

void rx_addrload( struct RexxHost *, struct rxd_addrload **, long, struct RexxMsg * );

struct rxd_addrnew
{
	long rc, rc2;
	struct {
		char *var, *stem;
		char *type;
		char *alias;
		char *name;
		char *email;
	} arg;
	struct {
		char *alias;
	} res;
};

void rx_addrnew( struct RexxHost *, struct rxd_addrnew **, long, struct RexxMsg * );

struct rxd_addrresolve
{
	long rc, rc2;
	struct {
		char *var, *stem;
		char *alias;
	} arg;
	struct {
		char *recpt;
	} res;
};

void rx_addrresolve( struct RexxHost *, struct rxd_addrresolve **, long, struct RexxMsg * );

struct rxd_addrsave
{
	long rc, rc2;
	struct {
		char *filename;
	} arg;
};

void rx_addrsave( struct RexxHost *, struct rxd_addrsave **, long, struct RexxMsg * );

struct rxd_appbusy
{
	long rc, rc2;
	struct {
		char *text;
	} arg;
};

void rx_appbusy( struct RexxHost *, struct rxd_appbusy **, long, struct RexxMsg * );

struct rxd_appnobusy
{
	long rc, rc2;
};

void rx_appnobusy( struct RexxHost *, struct rxd_appnobusy **, long, struct RexxMsg * );

struct rxd_folderinfo
{
	long rc, rc2;
	struct {
		char *var, *stem;
		char *folder;
	} arg;
	struct {
		long *number;
		char *name;
		char *path;
		long *total;
		long *new;
		long *unread;
		long *size;
		long *type;
	} res;
};

void rx_folderinfo( struct RexxHost *, struct rxd_folderinfo **, long, struct RexxMsg * );

struct rxd_getconfiginfo
{
	long rc, rc2;
	struct {
		char *var, *stem;
		char *item;
	} arg;
	struct {
		char *value;
	} res;
};

void rx_getconfiginfo( struct RexxHost *, struct rxd_getconfiginfo **, long, struct RexxMsg * );

struct rxd_getfolderinfo
{
	long rc, rc2;
	struct {
		char *var, *stem;
		char *item;
	} arg;
	struct {
		char *value;
	} res;
};

void rx_getfolderinfo( struct RexxHost *, struct rxd_getfolderinfo **, long, struct RexxMsg * );

struct rxd_getmailinfo
{
	long rc, rc2;
	struct {
		char *var, *stem;
		char *item;
	} arg;
	struct {
		char *value;
	} res;
};

void rx_getmailinfo( struct RexxHost *, struct rxd_getmailinfo **, long, struct RexxMsg * );

struct rxd_getselected
{
	long rc, rc2;
	struct {
		char *var, *stem;
	} arg;
	struct {
		long **num;
	} res;
};

void rx_getselected( struct RexxHost *, struct rxd_getselected **, long, struct RexxMsg * );

struct rxd_geturl
{
	long rc, rc2;
	struct {
		char *url;
		char *filename;
	} arg;
};

void rx_geturl( struct RexxHost *, struct rxd_geturl **, long, struct RexxMsg * );

struct rxd_help
{
	long rc, rc2;
	struct {
		char *file;
	} arg;
};

void rx_help( struct RexxHost *, struct rxd_help **, long, struct RexxMsg * );

struct rxd_hide
{
	long rc, rc2;
};

void rx_hide( struct RexxHost *, struct rxd_hide **, long, struct RexxMsg * );

struct rxd_info
{
	long rc, rc2;
	struct {
		char *var, *stem;
		char *item;
	} arg;
	struct {
		char *value;
	} res;
};

void rx_info( struct RexxHost *, struct rxd_info **, long, struct RexxMsg * );

struct rxd_isonline
{
	long rc, rc2;
};

void rx_isonline( struct RexxHost *, struct rxd_isonline **, long, struct RexxMsg * );

struct rxd_listselect
{
	long rc, rc2;
	struct {
		char *mode;
	} arg;
};

void rx_listselect( struct RexxHost *, struct rxd_listselect **, long, struct RexxMsg * );

struct rxd_mailarchive
{
	long rc, rc2;
	struct {
		char *folder;
	} arg;
};

void rx_mailarchive( struct RexxHost *, struct rxd_mailarchive **, long, struct RexxMsg * );

struct rxd_mailbounce
{
	long rc, rc2;
	struct {
		char *var, *stem;
		long quiet;
	} arg;
	struct {
		long *window;
	} res;
};

void rx_mailbounce( struct RexxHost *, struct rxd_mailbounce **, long, struct RexxMsg * );

struct rxd_mailchangesubject
{
	long rc, rc2;
	struct {
		char *subject;
	} arg;
};

void rx_mailchangesubject( struct RexxHost *, struct rxd_mailchangesubject **, long, struct RexxMsg * );

struct rxd_mailcheck
{
	long rc, rc2;
	struct {
		char *var, *stem;
		long *pop;
		long manual;
	} arg;
	struct {
		long *downloaded;
		long *onserver;
		long *dupskipped;
		long *deleted;
	} res;
};

void rx_mailcheck( struct RexxHost *, struct rxd_mailcheck **, long, struct RexxMsg * );

struct rxd_mailcopy
{
	long rc, rc2;
	struct {
		char *folder;
	} arg;
};

void rx_mailcopy( struct RexxHost *, struct rxd_mailcopy **, long, struct RexxMsg * );

struct rxd_maildelete
{
	long rc, rc2;
	struct {
		long atonce;
		long force;
	} arg;
};

void rx_maildelete( struct RexxHost *, struct rxd_maildelete **, long, struct RexxMsg * );

struct rxd_mailedit
{
	long rc, rc2;
	struct {
		char *var, *stem;
		long quiet;
	} arg;
	struct {
		long *window;
	} res;
};

void rx_mailedit( struct RexxHost *, struct rxd_mailedit **, long, struct RexxMsg * );

struct rxd_mailexport
{
	long rc, rc2;
	struct {
		char *filename;
		long all;
		long append;
	} arg;
};

void rx_mailexport( struct RexxHost *, struct rxd_mailexport **, long, struct RexxMsg * );

struct rxd_mailfilter
{
	long rc, rc2;
	struct {
		char *var, *stem;
		long all;
	} arg;
	struct {
		long *checked;
		long *bounced;
		long *forwarded;
		long *replied;
		long *executed;
		long *moved;
		long *deleted;
	} res;
};

void rx_mailfilter( struct RexxHost *, struct rxd_mailfilter **, long, struct RexxMsg * );

struct rxd_mailforward
{
	long rc, rc2;
	struct {
		char *var, *stem;
		long quiet;
	} arg;
	struct {
		long *window;
	} res;
};

void rx_mailforward( struct RexxHost *, struct rxd_mailforward **, long, struct RexxMsg * );

struct rxd_mailimport
{
	long rc, rc2;
	struct {
		char *filename;
		long wait;
	} arg;
};

void rx_mailimport( struct RexxHost *, struct rxd_mailimport **, long, struct RexxMsg * );

struct rxd_mailinfo
{
	long rc, rc2;
	struct {
		char *var, *stem;
		long *index;
	} arg;
	struct {
		long *index;
		char *status;
		char *from;
		char *to;
		char *replyto;
		char *subject;
		char *filename;
		long *size;
		char *date;
		char *flags;
		long *msgid;
	} res;
};

void rx_mailinfo( struct RexxHost *, struct rxd_mailinfo **, long, struct RexxMsg * );

struct rxd_mailmove
{
	long rc, rc2;
	struct {
		char *folder;
	} arg;
};

void rx_mailmove( struct RexxHost *, struct rxd_mailmove **, long, struct RexxMsg * );

struct rxd_mailread
{
	long rc, rc2;
	struct {
		char *var, *stem;
		long *window;
		long quiet;
	} arg;
	struct {
		long *window;
	} res;
};

void rx_mailread( struct RexxHost *, struct rxd_mailread **, long, struct RexxMsg * );

struct rxd_mailreply
{
	long rc, rc2;
	struct {
		char *var, *stem;
		long quiet;
	} arg;
	struct {
		long *window;
	} res;
};

void rx_mailreply( struct RexxHost *, struct rxd_mailreply **, long, struct RexxMsg * );

struct rxd_mailsend
{
	long rc, rc2;
	struct {
		long all;
	} arg;
};

void rx_mailsend( struct RexxHost *, struct rxd_mailsend **, long, struct RexxMsg * );

struct rxd_mailsendall
{
	long rc, rc2;
};

void rx_mailsendall( struct RexxHost *, struct rxd_mailsendall **, long, struct RexxMsg * );

struct rxd_mailstatus
{
	long rc, rc2;
	struct {
		char *status;
	} arg;
};

void rx_mailstatus( struct RexxHost *, struct rxd_mailstatus **, long, struct RexxMsg * );

struct rxd_mailupdate
{
	long rc, rc2;
};

void rx_mailupdate( struct RexxHost *, struct rxd_mailupdate **, long, struct RexxMsg * );

struct rxd_mailwrite
{
	long rc, rc2;
	struct {
		char *var, *stem;
		long *window;
		long quiet;
	} arg;
	struct {
		long *window;
	} res;
};

void rx_mailwrite( struct RexxHost *, struct rxd_mailwrite **, long, struct RexxMsg * );

struct rxd_newmailfile
{
	long rc, rc2;
	struct {
		char *var, *stem;
		char *folder;
	} arg;
	struct {
		char *filename;
	} res;
};

void rx_newmailfile( struct RexxHost *, struct rxd_newmailfile **, long, struct RexxMsg * );

struct rxd_quit
{
	long rc, rc2;
	struct {
		long force;
	} arg;
};

void rx_quit( struct RexxHost *, struct rxd_quit **, long, struct RexxMsg * );

struct rxd_readclose
{
	long rc, rc2;
};

void rx_readclose( struct RexxHost *, struct rxd_readclose **, long, struct RexxMsg * );

struct rxd_readinfo
{
	long rc, rc2;
	struct {
		char *var, *stem;
	} arg;
	struct {
		char **filename;
		char **filetype;
		long **filesize;
		char **tempfile;
	} res;
};

void rx_readinfo( struct RexxHost *, struct rxd_readinfo **, long, struct RexxMsg * );

struct rxd_readprint
{
	long rc, rc2;
	struct {
		long *part;
	} arg;
};

void rx_readprint( struct RexxHost *, struct rxd_readprint **, long, struct RexxMsg * );

struct rxd_readsave
{
	long rc, rc2;
	struct {
		long *part;
		char *filename;
		long overwrite;
	} arg;
};

void rx_readsave( struct RexxHost *, struct rxd_readsave **, long, struct RexxMsg * );

struct rxd_request
{
	long rc, rc2;
	struct {
		char *var, *stem;
		char *body;
		char *gadgets;
	} arg;
	struct {
		long *result;
	} res;
};

void rx_request( struct RexxHost *, struct rxd_request **, long, struct RexxMsg * );

struct rxd_requestfolder
{
	long rc, rc2;
	struct {
		char *var, *stem;
		char *body;
		long excludeactive;
	} arg;
	struct {
		char *folder;
	} res;
};

void rx_requestfolder( struct RexxHost *, struct rxd_requestfolder **, long, struct RexxMsg * );

struct rxd_requeststring
{
	long rc, rc2;
	struct {
		char *var, *stem;
		char *body;
		char *string;
		long secret;
	} arg;
	struct {
		char *string;
	} res;
};

void rx_requeststring( struct RexxHost *, struct rxd_requeststring **, long, struct RexxMsg * );

struct rxd_screentoback
{
	long rc, rc2;
};

void rx_screentoback( struct RexxHost *, struct rxd_screentoback **, long, struct RexxMsg * );

struct rxd_screentofront
{
	long rc, rc2;
};

void rx_screentofront( struct RexxHost *, struct rxd_screentofront **, long, struct RexxMsg * );

struct rxd_setflag
{
	long rc, rc2;
	struct {
		long *vol;
		long *per;
	} arg;
};

void rx_setflag( struct RexxHost *, struct rxd_setflag **, long, struct RexxMsg * );

struct rxd_setfolder
{
	long rc, rc2;
	struct {
		char *folder;
	} arg;
};

void rx_setfolder( struct RexxHost *, struct rxd_setfolder **, long, struct RexxMsg * );

struct rxd_setmail
{
	long rc, rc2;
	struct {
		long *num;
	} arg;
};

void rx_setmail( struct RexxHost *, struct rxd_setmail **, long, struct RexxMsg * );

struct rxd_setmailfile
{
	long rc, rc2;
	struct {
		char *mailfile;
	} arg;
};

void rx_setmailfile( struct RexxHost *, struct rxd_setmailfile **, long, struct RexxMsg * );

struct rxd_show
{
	long rc, rc2;
};

void rx_show( struct RexxHost *, struct rxd_show **, long, struct RexxMsg * );

struct rxd_userinfo
{
	long rc, rc2;
	struct {
		char *var, *stem;
	} arg;
	struct {
		char *username;
		char *email;
		char *realname;
		char *config;
		char *maildir;
		long *folders;
	} res;
};

void rx_userinfo( struct RexxHost *, struct rxd_userinfo **, long, struct RexxMsg * );

struct rxd_writeattach
{
	long rc, rc2;
	struct {
		char *file;
		char *desc;
		char *encmode;
		char *ctype;
	} arg;
};

void rx_writeattach( struct RexxHost *, struct rxd_writeattach **, long, struct RexxMsg * );

struct rxd_writebcc
{
	long rc, rc2;
	struct {
		char **address;
		long add;
	} arg;
};

void rx_writebcc( struct RexxHost *, struct rxd_writebcc **, long, struct RexxMsg * );

struct rxd_writecc
{
	long rc, rc2;
	struct {
		char **address;
		long add;
	} arg;
};

void rx_writecc( struct RexxHost *, struct rxd_writecc **, long, struct RexxMsg * );

struct rxd_writeeditor
{
	long rc, rc2;
	struct {
		char *var, *stem;
		char *command;
	} arg;
	struct {
		char *result;
	} res;
};

void rx_writeeditor( struct RexxHost *, struct rxd_writeeditor **, long, struct RexxMsg * );

struct rxd_writefrom
{
	long rc, rc2;
	struct {
		char *address;
	} arg;
};

void rx_writefrom( struct RexxHost *, struct rxd_writefrom **, long, struct RexxMsg * );

struct rxd_writeletter
{
	long rc, rc2;
	struct {
		char *file;
		long nosig;
	} arg;
};

void rx_writeletter( struct RexxHost *, struct rxd_writeletter **, long, struct RexxMsg * );

struct rxd_writemailto
{
	long rc, rc2;
	struct {
		char **address;
	} arg;
};

void rx_writemailto( struct RexxHost *, struct rxd_writemailto **, long, struct RexxMsg * );

struct rxd_writeoptions
{
	long rc, rc2;
	struct {
		long delete;
		long receipt;
		long notif;
		long addinfo;
		long *importance;
		long *sig;
		long *security;
	} arg;
};

void rx_writeoptions( struct RexxHost *, struct rxd_writeoptions **, long, struct RexxMsg * );

struct rxd_writequeue
{
	long rc, rc2;
	struct {
		long hold;
	} arg;
};

void rx_writequeue( struct RexxHost *, struct rxd_writequeue **, long, struct RexxMsg * );

struct rxd_writereplyto
{
	long rc, rc2;
	struct {
		char *address;
	} arg;
};

void rx_writereplyto( struct RexxHost *, struct rxd_writereplyto **, long, struct RexxMsg * );

struct rxd_writesend
{
	long rc, rc2;
};

void rx_writesend( struct RexxHost *, struct rxd_writesend **, long, struct RexxMsg * );

struct rxd_writesubject
{
	long rc, rc2;
	struct {
		char *subject;
	} arg;
};

void rx_writesubject( struct RexxHost *, struct rxd_writesubject **, long, struct RexxMsg * );

struct rxd_writeto
{
	long rc, rc2;
	struct {
		char **address;
		long add;
	} arg;
};

void rx_writeto( struct RexxHost *, struct rxd_writeto **, long, struct RexxMsg * );

#endif
