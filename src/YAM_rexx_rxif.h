#ifndef YAM_REXX_RXIF_H
#define YAM_REXX_RXIF_H

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
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 YAM Official Support Site :  http://www.yam.ch
 YAM OpenSource project    :  http://sourceforge.net/projects/yamos/

 $Id$

***************************************************************************/

#include <rexx/storage.h>

enum RexxAction
{
  RXIF_INIT   = 1,
  RXIF_ACTION = 2,
  RXIF_FREE   = 3
};

struct RexxHost
{
   struct MsgPort *port;
   char portname[ 80 ];
   int portnumber;
   long replies;
   struct RDArgs *rdargs;
   long flags;
   APTR userdata;
};

struct RuleResult
{
   long Checked;
   long Bounced;
   long Forwarded;
   long Replied;
   long Executed;
   long Moved;
   long Deleted;
   long Spam;
};

/* Add local variables only at end of rxd structs */

struct rxd_addrdelete
{
   long rc, rc2;
   struct {
      char *alias;
   } arg;
};
void rx_addrdelete( struct RexxHost *, struct rxd_addrdelete **, enum RexxAction, struct RexxMsg * );

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

void rx_addredit( struct RexxHost *, struct rxd_addredit **, enum RexxAction, struct RexxMsg * );

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

void rx_addrfind( struct RexxHost *, struct rxd_addrfind **, enum RexxAction, struct RexxMsg * );

struct rxd_addrgoto
{
        long rc, rc2;
        struct {
                char *alias;
        } arg;
};

void rx_addrgoto( struct RexxHost *, struct rxd_addrgoto **, enum RexxAction, struct RexxMsg * );

struct rxd_addrinfo
{
  long rc, rc2;
  struct
  {
    char *var, *stem;
    char *alias;
  } arg;
  struct
  {
    const char *type;
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

void rx_addrinfo( struct RexxHost *, struct rxd_addrinfo **, enum RexxAction, struct RexxMsg * );

struct rxd_addrload
{
        long rc, rc2;
        struct {
                char *filename;
        } arg;
};

void rx_addrload( struct RexxHost *, struct rxd_addrload **, enum RexxAction, struct RexxMsg * );

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

void rx_addrnew( struct RexxHost *, struct rxd_addrnew **, enum RexxAction, struct RexxMsg * );

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

void rx_addrresolve( struct RexxHost *, struct rxd_addrresolve **, enum RexxAction, struct RexxMsg * );

struct rxd_addrsave
{
        long rc, rc2;
        struct {
                char *filename;
        } arg;
};

void rx_addrsave( struct RexxHost *, struct rxd_addrsave **, enum RexxAction, struct RexxMsg * );

struct rxd_appbusy
{
        long rc, rc2;
        struct {
                char *text;
        } arg;
};

void rx_appbusy( struct RexxHost *, struct rxd_appbusy **, enum RexxAction, struct RexxMsg * );

struct rxd_appnobusy
{
        long rc, rc2;
};

void rx_appnobusy( struct RexxHost *, struct rxd_appnobusy **, enum RexxAction, struct RexxMsg * );

struct rxd_folderinfo
{
        long rc, rc2;
        struct {
                char *var, *stem;
                char *folder;
        } arg;
        struct {
                int *number;
                char *name;
                char *path;
                int *total;
                int *new;
                int *unread;
                LONG *size;
                int *type;
        } res;
};

void rx_folderinfo( struct RexxHost *, struct rxd_folderinfo **, enum RexxAction, struct RexxMsg * );

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

void rx_getconfiginfo( struct RexxHost *, struct rxd_getconfiginfo **, enum RexxAction, struct RexxMsg * );

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

void rx_getfolderinfo( struct RexxHost *, struct rxd_getfolderinfo **, enum RexxAction, struct RexxMsg * );

struct rxd_getmailinfo
{
  long rc, rc2;
  struct
  {
    char *var, *stem;
    char *item;
  } arg;
  struct
  {
    const char *value;
  } res;
};

void rx_getmailinfo( struct RexxHost *, struct rxd_getmailinfo **, enum RexxAction, struct RexxMsg * );

struct rxd_getselected
{
        long rc, rc2;
        struct {
                char *var, *stem;
        } arg;
        struct {
                int **num;
        } res;
};

void rx_getselected( struct RexxHost *, struct rxd_getselected **, enum RexxAction, struct RexxMsg * );

struct rxd_geturl
{
        long rc, rc2;
        struct {
                char *url;
                char *filename;
        } arg;
};

void rx_geturl( struct RexxHost *, struct rxd_geturl **, enum RexxAction, struct RexxMsg * );

struct rxd_help
{
        long rc, rc2;
        struct {
                char *file;
        } arg;
};

void rx_help( struct RexxHost *, struct rxd_help **, enum RexxAction, struct RexxMsg * );

struct rxd_hide
{
        long rc, rc2;
};

void rx_hide( struct RexxHost *, struct rxd_hide **, enum RexxAction, struct RexxMsg * );

struct rxd_info
{
  long rc, rc2;
  struct
  {
    char *var, *stem;
    char *item;
  } arg;
  struct
  {
    const char *value;
  } res;
};

void rx_info( struct RexxHost *, struct rxd_info **, enum RexxAction, struct RexxMsg * );

struct rxd_isonline
{
        long rc, rc2;
};

void rx_isonline( struct RexxHost *, struct rxd_isonline **, enum RexxAction, struct RexxMsg * );

struct rxd_listselect
{
        long rc, rc2;
        struct {
                char *mode;
        } arg;
};

void rx_listselect( struct RexxHost *, struct rxd_listselect **, enum RexxAction, struct RexxMsg * );

struct rxd_mailarchive
{
        long rc, rc2;
        struct {
                char *folder;
        } arg;
};

void rx_mailarchive( struct RexxHost *, struct rxd_mailarchive **, enum RexxAction, struct RexxMsg * );

struct rxd_mailbounce
{
        long rc, rc2;
        struct {
                char *var, *stem;
                long quiet;
        } arg;
        struct {
                int *window;
        } res;
};

void rx_mailbounce( struct RexxHost *, struct rxd_mailbounce **, enum RexxAction, struct RexxMsg * );

struct rxd_mailchangesubject
{
        long rc, rc2;
        struct {
                char *subject;
        } arg;
};

void rx_mailchangesubject( struct RexxHost *, struct rxd_mailchangesubject **, enum RexxAction, struct RexxMsg * );

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

void rx_mailcheck( struct RexxHost *, struct rxd_mailcheck **, enum RexxAction, struct RexxMsg * );

struct rxd_mailcopy
{
        long rc, rc2;
        struct {
                char *folder;
        } arg;
};

void rx_mailcopy( struct RexxHost *, struct rxd_mailcopy **, enum RexxAction, struct RexxMsg * );

struct rxd_maildelete
{
        long rc, rc2;
        struct {
                long atonce;
                long force;
        } arg;
};

void rx_maildelete( struct RexxHost *, struct rxd_maildelete **, enum RexxAction, struct RexxMsg * );

struct rxd_mailedit
{
        long rc, rc2;
        struct {
                char *var, *stem;
                long quiet;
        } arg;
        struct {
                int *window;
        } res;
};

void rx_mailedit( struct RexxHost *, struct rxd_mailedit **, enum RexxAction, struct RexxMsg * );

struct rxd_mailexport
{
        long rc, rc2;
        struct {
                char *filename;
                long all;
                long append;
        } arg;
};

void rx_mailexport( struct RexxHost *, struct rxd_mailexport **, enum RexxAction, struct RexxMsg * );

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
                long *spam;
        } res;
};

void rx_mailfilter( struct RexxHost *, struct rxd_mailfilter **, enum RexxAction, struct RexxMsg * );

struct rxd_mailforward
{
        long rc, rc2;
        struct {
                char *var, *stem;
                long quiet;
        } arg;
        struct {
                int *window;
        } res;
};

void rx_mailforward( struct RexxHost *, struct rxd_mailforward **, enum RexxAction, struct RexxMsg * );

struct rxd_mailimport
{
        long rc, rc2;
        struct {
                char *filename;
                long wait;
        } arg;
};

void rx_mailimport( struct RexxHost *, struct rxd_mailimport **, enum RexxAction, struct RexxMsg * );

struct rxd_mailinfo
{
  long rc, rc2;
  struct
  {
    char *var, *stem;
    long *index;
  } arg;
  struct
  {
    long *index;
    const char *status;
    char *from;
    char *to;
    char *replyto;
    char *subject;
    char *filename;
    long *size;
    char *date;
    char *flags;
    char *msgid;
  } res;
};

void rx_mailinfo( struct RexxHost *, struct rxd_mailinfo **, enum RexxAction, struct RexxMsg * );

struct rxd_mailmove
{
        long rc, rc2;
        struct {
                char *folder;
        } arg;
};

void rx_mailmove( struct RexxHost *, struct rxd_mailmove **, enum RexxAction, struct RexxMsg * );

struct rxd_mailread
{
        long rc, rc2;
        struct {
                char *var, *stem;
                int *window;
                long quiet;
        } arg;
        struct {
                int *window;
        } res;
};

void rx_mailread( struct RexxHost *, struct rxd_mailread **, enum RexxAction, struct RexxMsg * );

struct rxd_mailreply
{
        long rc, rc2;
        struct {
                char *var, *stem;
                long quiet;
        } arg;
        struct {
                int *window;
        } res;
};

void rx_mailreply( struct RexxHost *, struct rxd_mailreply **, enum RexxAction, struct RexxMsg * );

struct rxd_mailsend
{
        long rc, rc2;
        struct {
                long all;
        } arg;
};

void rx_mailsend( struct RexxHost *, struct rxd_mailsend **, enum RexxAction, struct RexxMsg * );

struct rxd_mailsendall
{
        long rc, rc2;
};

void rx_mailsendall( struct RexxHost *, struct rxd_mailsendall **, enum RexxAction, struct RexxMsg * );

struct rxd_mailstatus
{
        long rc, rc2;
        struct {
                char *status;
        } arg;
};

void rx_mailstatus( struct RexxHost *, struct rxd_mailstatus **, enum RexxAction, struct RexxMsg * );

struct rxd_mailupdate
{
        long rc, rc2;
};

void rx_mailupdate( struct RexxHost *, struct rxd_mailupdate **, enum RexxAction, struct RexxMsg * );

struct rxd_mailwrite
{
        long rc, rc2;
        struct {
                char *var, *stem;
                int *window;
                long quiet;
        } arg;
        struct {
                int *window;
        } res;
};

void rx_mailwrite( struct RexxHost *, struct rxd_mailwrite **, enum RexxAction, struct RexxMsg * );

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

void rx_newmailfile( struct RexxHost *, struct rxd_newmailfile **, enum RexxAction, struct RexxMsg * );

struct rxd_quit
{
        long rc, rc2;
        struct {
                long force;
        } arg;
};

void rx_quit( struct RexxHost *, struct rxd_quit **, enum RexxAction, struct RexxMsg * );

struct rxd_readclose
{
        long rc, rc2;
};

void rx_readclose( struct RexxHost *, struct rxd_readclose **, enum RexxAction, struct RexxMsg * );

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

void rx_readinfo( struct RexxHost *, struct rxd_readinfo **, enum RexxAction, struct RexxMsg * );

struct rxd_readprint
{
        long rc, rc2;
        struct {
                long *part;
        } arg;
};

void rx_readprint( struct RexxHost *, struct rxd_readprint **, enum RexxAction, struct RexxMsg * );

struct rxd_readsave
{
        long rc, rc2;
        struct {
                long *part;
                char *filename;
                long overwrite;
        } arg;
};

void rx_readsave( struct RexxHost *, struct rxd_readsave **, enum RexxAction, struct RexxMsg * );

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

void rx_request( struct RexxHost *, struct rxd_request **, enum RexxAction, struct RexxMsg * );

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

void rx_requestfolder( struct RexxHost *, struct rxd_requestfolder **, enum RexxAction, struct RexxMsg * );

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

void rx_requeststring( struct RexxHost *, struct rxd_requeststring **, enum RexxAction, struct RexxMsg * );

struct rxd_screentoback
{
        long rc, rc2;
};

void rx_screentoback( struct RexxHost *, struct rxd_screentoback **, enum RexxAction, struct RexxMsg * );

struct rxd_screentofront
{
        long rc, rc2;
};

void rx_screentofront( struct RexxHost *, struct rxd_screentofront **, enum RexxAction, struct RexxMsg * );

struct rxd_setflag
{
        long rc, rc2;
        struct {
                long *vol;
                long *per;
        } arg;
};

void rx_setflag( struct RexxHost *, struct rxd_setflag **, enum RexxAction, struct RexxMsg * );

struct rxd_setfolder
{
        long rc, rc2;
        struct {
                char *folder;
        } arg;
};

void rx_setfolder( struct RexxHost *, struct rxd_setfolder **, enum RexxAction, struct RexxMsg * );

struct rxd_setmail
{
        long rc, rc2;
        struct {
                long *num;
        } arg;
};

void rx_setmail( struct RexxHost *, struct rxd_setmail **, enum RexxAction, struct RexxMsg * );

struct rxd_setmailfile
{
        long rc, rc2;
        struct {
                char *mailfile;
        } arg;
};

void rx_setmailfile( struct RexxHost *, struct rxd_setmailfile **, enum RexxAction, struct RexxMsg * );

struct rxd_show
{
        long rc, rc2;
};

void rx_show( struct RexxHost *, struct rxd_show **, enum RexxAction, struct RexxMsg * );

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

void rx_userinfo( struct RexxHost *, struct rxd_userinfo **, enum RexxAction, struct RexxMsg * );

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

void rx_writeattach( struct RexxHost *, struct rxd_writeattach **, enum RexxAction, struct RexxMsg * );

struct rxd_writebcc
{
        long rc, rc2;
        struct {
                char **address;
                long add;
        } arg;
};

void rx_writebcc( struct RexxHost *, struct rxd_writebcc **, enum RexxAction, struct RexxMsg * );

struct rxd_writecc
{
        long rc, rc2;
        struct {
                char **address;
                long add;
        } arg;
};

void rx_writecc( struct RexxHost *, struct rxd_writecc **, enum RexxAction, struct RexxMsg * );

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

void rx_writeeditor( struct RexxHost *, struct rxd_writeeditor **, enum RexxAction, struct RexxMsg * );

struct rxd_writefrom
{
        long rc, rc2;
        struct {
                char *address;
        } arg;
};

void rx_writefrom( struct RexxHost *, struct rxd_writefrom **, enum RexxAction, struct RexxMsg * );

struct rxd_writeletter
{
        long rc, rc2;
        struct {
                char *file;
                long nosig;
        } arg;
};

void rx_writeletter( struct RexxHost *, struct rxd_writeletter **, enum RexxAction, struct RexxMsg * );

struct rxd_writemailto
{
        long rc, rc2;
        struct {
                char **address;
        } arg;
};

void rx_writemailto( struct RexxHost *, struct rxd_writemailto **, enum RexxAction, struct RexxMsg * );

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

void rx_writeoptions( struct RexxHost *, struct rxd_writeoptions **, enum RexxAction, struct RexxMsg * );

struct rxd_writequeue
{
        long rc, rc2;
        struct {
                long hold;
        } arg;
};

void rx_writequeue( struct RexxHost *, struct rxd_writequeue **, enum RexxAction, struct RexxMsg * );

struct rxd_writereplyto
{
        long rc, rc2;
        struct {
                char *address;
        } arg;
};

void rx_writereplyto( struct RexxHost *, struct rxd_writereplyto **, enum RexxAction, struct RexxMsg * );

struct rxd_writesend
{
        long rc, rc2;
};

void rx_writesend( struct RexxHost *, struct rxd_writesend **, enum RexxAction, struct RexxMsg * );

struct rxd_writesubject
{
        long rc, rc2;
        struct {
                char *subject;
        } arg;
};

void rx_writesubject( struct RexxHost *, struct rxd_writesubject **, enum RexxAction, struct RexxMsg * );

struct rxd_writeto
{
        long rc, rc2;
        struct {
                char **address;
                long add;
        } arg;
};

void rx_writeto( struct RexxHost *, struct rxd_writeto **, enum RexxAction, struct RexxMsg * );

#endif /* YAM_REXX_RXIF_H */
