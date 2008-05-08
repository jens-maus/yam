#ifndef REXX_H
#define REXX_H

/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2008 by YAM Open Source Team

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

// forward declarations
struct RexxMsg;
struct MsgPort;
struct RDArgs;

struct RexxHost
{
   struct MsgPort *port;
   char portname[80];
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

void ARexxDispatch(struct RexxHost *host);
void CloseDownARexxHost(struct RexxHost *host);
void DoRXCommand(struct RexxHost *host, struct RexxMsg *rexxmsg);
void FreeRexxCommand(struct RexxMsg *rxmsg);
void ReplyRexxCommand(struct RexxMsg *rxmsg, long prim, long sec, char *res);
struct RexxMsg *SendRexxCommand(struct RexxHost *host, char *buff, BPTR fh);
struct RexxHost *SetupARexxHost(const char *basename, struct MsgPort *usrport);

enum RexxAction
{
  RXIF_INIT   = 1,
  RXIF_ACTION = 2,
  RXIF_FREE   = 3
};

struct rxs_command
{
  const char *command;
  const char *args;
  const char *results;
  long resindex;
  void (*function)(struct RexxHost *, void **, enum RexxAction, struct RexxMsg *);
  long flags;
};

extern struct rxs_command rxs_commandlist[];

void rx_addrdelete(struct RexxHost *, void **, enum RexxAction, struct RexxMsg *);
void rx_addredit(struct RexxHost *, void **, enum RexxAction, struct RexxMsg *);
void rx_addrfind(struct RexxHost *, void **, enum RexxAction, struct RexxMsg *);
void rx_addrgoto(struct RexxHost *, void **, enum RexxAction, struct RexxMsg *);
void rx_addrinfo(struct RexxHost *, void **, enum RexxAction, struct RexxMsg *);
void rx_addrload(struct RexxHost *, void **, enum RexxAction, struct RexxMsg *);
void rx_addrnew(struct RexxHost *, void **, enum RexxAction, struct RexxMsg *);
void rx_addrresolve(struct RexxHost *, void **, enum RexxAction, struct RexxMsg *);
void rx_addrsave(struct RexxHost *, void **, enum RexxAction, struct RexxMsg *);
void rx_appbusy(struct RexxHost *, void **, enum RexxAction, struct RexxMsg *);
void rx_appnobusy(struct RexxHost *, void **, enum RexxAction, struct RexxMsg *);
void rx_folderinfo(struct RexxHost *, void **, enum RexxAction, struct RexxMsg *);
void rx_getconfiginfo(struct RexxHost *, void **, enum RexxAction, struct RexxMsg *);
void rx_getfolderinfo(struct RexxHost *, void **, enum RexxAction, struct RexxMsg *);
void rx_getmailinfo(struct RexxHost *, void **, enum RexxAction, struct RexxMsg *);
void rx_getselected(struct RexxHost *, void **, enum RexxAction, struct RexxMsg *);
void rx_geturl(struct RexxHost *, void **, enum RexxAction, struct RexxMsg *);
void rx_help(struct RexxHost *, void **, enum RexxAction, struct RexxMsg *);
void rx_hide(struct RexxHost *, void **, enum RexxAction, struct RexxMsg *);
void rx_info(struct RexxHost *, void **, enum RexxAction, struct RexxMsg *);
void rx_isonline(struct RexxHost *, void **, enum RexxAction, struct RexxMsg *);
void rx_listselect(struct RexxHost *, void **, enum RexxAction, struct RexxMsg *);
void rx_mailarchive(struct RexxHost *, void **, enum RexxAction, struct RexxMsg *);
void rx_mailbounce(struct RexxHost *, void **, enum RexxAction, struct RexxMsg *);
void rx_mailchangesubject(struct RexxHost *, void **, enum RexxAction, struct RexxMsg *);
void rx_mailcheck(struct RexxHost *, void **, enum RexxAction, struct RexxMsg *);
void rx_mailcopy(struct RexxHost *, void **, enum RexxAction, struct RexxMsg *);
void rx_maildelete(struct RexxHost *, void **, enum RexxAction, struct RexxMsg *);
void rx_mailedit(struct RexxHost *, void **, enum RexxAction, struct RexxMsg *);
void rx_mailexport(struct RexxHost *, void **, enum RexxAction, struct RexxMsg *);
void rx_mailfilter(struct RexxHost *, void **, enum RexxAction, struct RexxMsg *);
void rx_mailforward(struct RexxHost *, void **, enum RexxAction, struct RexxMsg *);
void rx_mailimport(struct RexxHost *, void **, enum RexxAction, struct RexxMsg *);
void rx_mailinfo(struct RexxHost *, void **, enum RexxAction, struct RexxMsg *);
void rx_mailmove(struct RexxHost *, void **, enum RexxAction, struct RexxMsg *);
void rx_mailread(struct RexxHost *, void **, enum RexxAction, struct RexxMsg *);
void rx_mailreply(struct RexxHost *, void **, enum RexxAction, struct RexxMsg *);
void rx_mailsend(struct RexxHost *, void **, enum RexxAction, struct RexxMsg *);
void rx_mailsendall(struct RexxHost *, void **, enum RexxAction, struct RexxMsg *);
void rx_mailstatus(struct RexxHost *, void **, enum RexxAction, struct RexxMsg *);
void rx_mailupdate(struct RexxHost *, void **, enum RexxAction, struct RexxMsg *);
void rx_mailwrite(struct RexxHost *, void **, enum RexxAction, struct RexxMsg *);
void rx_newmailfile(struct RexxHost *, void **, enum RexxAction, struct RexxMsg *);
void rx_quit(struct RexxHost *, void **, enum RexxAction, struct RexxMsg *);
void rx_readclose(struct RexxHost *, void **, enum RexxAction, struct RexxMsg *);
void rx_readinfo(struct RexxHost *, void **, enum RexxAction, struct RexxMsg *);
void rx_readprint(struct RexxHost *, void **, enum RexxAction, struct RexxMsg *);
void rx_readsave(struct RexxHost *, void **, enum RexxAction, struct RexxMsg *);
void rx_request(struct RexxHost *, void **, enum RexxAction, struct RexxMsg *);
void rx_requestfolder(struct RexxHost *, void **, enum RexxAction, struct RexxMsg *);
void rx_requeststring(struct RexxHost *, void **, enum RexxAction, struct RexxMsg *);
void rx_screentoback(struct RexxHost *, void **, enum RexxAction, struct RexxMsg *);
void rx_screentofront(struct RexxHost *, void **, enum RexxAction, struct RexxMsg *);
void rx_setflag(struct RexxHost *, void **, enum RexxAction, struct RexxMsg *);
void rx_setfolder(struct RexxHost *, void **, enum RexxAction, struct RexxMsg *);
void rx_setmail(struct RexxHost *, void **, enum RexxAction, struct RexxMsg *);
void rx_setmailfile(struct RexxHost *, void **, enum RexxAction, struct RexxMsg *);
void rx_show(struct RexxHost *, void **, enum RexxAction, struct RexxMsg *);
void rx_userinfo(struct RexxHost *, void **, enum RexxAction, struct RexxMsg *);
void rx_writeattach(struct RexxHost *, void **, enum RexxAction, struct RexxMsg *);
void rx_writebcc(struct RexxHost *, void **, enum RexxAction, struct RexxMsg *);
void rx_writecc(struct RexxHost *, void **, enum RexxAction, struct RexxMsg *);
void rx_writeeditor(struct RexxHost *, void **, enum RexxAction, struct RexxMsg *);
void rx_writefrom(struct RexxHost *, void **, enum RexxAction, struct RexxMsg *);
void rx_writeletter(struct RexxHost *, void **, enum RexxAction, struct RexxMsg *);
void rx_writemailto(struct RexxHost *, void **, enum RexxAction, struct RexxMsg *);
void rx_writeoptions(struct RexxHost *, void **, enum RexxAction, struct RexxMsg *);
void rx_writequeue(struct RexxHost *, void **, enum RexxAction, struct RexxMsg *);
void rx_writereplyto(struct RexxHost *, void **, enum RexxAction, struct RexxMsg *);
void rx_writesend(struct RexxHost *, void **, enum RexxAction, struct RexxMsg *);
void rx_writesubject(struct RexxHost *, void **, enum RexxAction, struct RexxMsg *);
void rx_writeto(struct RexxHost *, void **, enum RexxAction, struct RexxMsg *);

struct rxd_addrdelete
{
  long rc, rc2;
  struct {
    char *alias;
  } arg;
};

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

struct rxd_addrgoto
{
  long rc, rc2;
  struct {
    char *alias;
  } arg;
};

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

struct rxd_addrload
{
  long rc, rc2;
  struct {
    char *filename;
  } arg;
};

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

struct rxd_addrresolve
{
  long rc, rc2;
  struct
  {
    char *var, *stem;
    char *alias;
  } arg;
  struct
  {
    char *recpt;
  } res;
};

struct rxd_addrsave
{
  long rc, rc2;
  struct {
    char *filename;
  } arg;
};

struct rxd_appbusy
{
  long rc, rc2;
  struct {
    char *text;
  } arg;
};

struct rxd_appnobusy
{
  long rc, rc2;
};

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

struct rxd_geturl
{
  long rc, rc2;
  struct {
    char *url;
    char *filename;
  } arg;
};

struct rxd_help
{
  long rc, rc2;
  struct {
    char *file;
  } arg;
};

struct rxd_hide
{
  long rc, rc2;
};

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

struct rxd_isonline
{
  long rc, rc2;
};

struct rxd_listselect
{
  long rc, rc2;
  struct {
    char *mode;
  } arg;
};

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

struct rxd_mailchangesubject
{
  long rc, rc2;
  struct {
    char *subject;
  } arg;
};

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

struct rxd_mailcopy
{
  long rc, rc2;
  struct {
    char *folder;
  } arg;
};

struct rxd_maildelete
{
  long rc, rc2;
  struct {
    long atonce;
    long force;
  } arg;
};

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

struct rxd_mailexport
{
  long rc, rc2;
  struct {
    char *filename;
    long all;
    long append;
  } arg;
};

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

struct rxd_mailimport
{
  long rc, rc2;
  struct {
    char *filename;
    long wait;
  } arg;
};

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

struct rxd_mailmove
{
  long rc, rc2;
  struct {
    char *folder;
  } arg;
};

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

struct rxd_mailsend
{
  long rc, rc2;
  struct {
    long all;
  } arg;
};

struct rxd_mailsendall
{
  long rc, rc2;
};

struct rxd_mailstatus
{
  long rc, rc2;
  struct {
    char *status;
  } arg;
};

struct rxd_mailupdate
{
  long rc, rc2;
};

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

struct rxd_quit
{
  long rc, rc2;
  struct {
    long force;
  } arg;
};

struct rxd_readclose
{
  long rc, rc2;
};

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

struct rxd_readprint
{
  long rc, rc2;
  struct {
    long *part;
  } arg;
};

struct rxd_readsave
{
  long rc, rc2;
  struct {
    long *part;
    char *filename;
    long overwrite;
  } arg;
};

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

struct rxd_screentoback
{
  long rc, rc2;
};

struct rxd_screentofront
{
  long rc, rc2;
};

struct rxd_setflag
{
  long rc, rc2;
  struct {
    long *vol;
    long *per;
  } arg;
};

struct rxd_setfolder
{
  long rc, rc2;
  struct
  {
    char *folder;
  } arg;
};

struct rxd_setmail
{
  long rc, rc2;
  struct
  {
    long *num;
  } arg;
};

struct rxd_setmailfile
{
  long rc, rc2;
  struct {
    char *mailfile;
  } arg;
};

struct rxd_show
{
  long rc, rc2;
};

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

struct rxd_writebcc
{
  long rc, rc2;
  struct {
    char **address;
    long add;
  } arg;
};

struct rxd_writecc
{
  long rc, rc2;
  struct {
    char **address;
    long add;
  } arg;
};

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

struct rxd_writefrom
{
  long rc, rc2;
  struct {
    char *address;
  } arg;
};

struct rxd_writeletter
{
  long rc, rc2;
  struct {
    char *file;
    long nosig;
  } arg;
};

struct rxd_writemailto
{
  long rc, rc2;
  struct {
    char **address;
  } arg;
};

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

struct rxd_writequeue
{
  long rc, rc2;
  struct {
    long hold;
  } arg;
};

struct rxd_writereplyto
{
  long rc, rc2;
  struct {
    char *address;
  } arg;
};

struct rxd_writesend
{
  long rc, rc2;
};

struct rxd_writesubject
{
  long rc, rc2;
  struct {
    char *subject;
  } arg;
};

struct rxd_writeto
{
  long rc, rc2;
  struct {
    char **address;
    long add;
  } arg;
};

#endif /* REXX_H */
