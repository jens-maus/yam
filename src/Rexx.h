#ifndef REXX_H
#define REXX_H

/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2019 YAM Open Source Team

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

void ARexxDispatch(struct RexxHost *host);
void CloseDownARexxHost(struct RexxHost *host);
void DoRXCommand(struct RexxHost *host, struct RexxMsg *rexxmsg);
void FreeRexxCommand(struct RexxMsg *rxmsg);
void ReplyRexxCommand(struct RexxMsg *rxmsg, long prim, long sec, char *res);
struct RexxMsg *SendRexxCommand(struct RexxHost *host, char *buff, BPTR fh);
struct RexxHost *SetupARexxHost(const char *basename, struct MsgPort *usrport);
BOOL SendToYAMInstance(char *rxcmd);

enum RexxAction
{
  RXIF_INIT   = 1,
  RXIF_ACTION = 2,
  RXIF_FREE   = 3
};

struct RexxParams
{
  long rc;
  long rc2;
  void *args;
  void *results;
  void *optional;
};

struct RexxResult
{
  char *var;
  char *stem;
};

struct rxs_command
{
  const char *command;
  const char *args;
  const char *results;
  void (*function)(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
};

extern struct rxs_command rxs_commandlist[];

void rx_addrdelete(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_addredit(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_addrfind(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_addrgoto(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_addrinfo(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_addrload(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_addrnew(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_addrresolve(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_addrsave(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_appbusy(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_appnobusy(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_findmail(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_flushindexes(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_folderinfo(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_getconfiginfo(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_getfolderinfo(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_getmailinfo(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_getselected(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_geturl(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_help(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_hide(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_info(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_isonline(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_listfreeze(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_listselect(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_listunfreeze(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_mailarchive(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_mailbounce(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_mailchangesubject(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_mailcheck(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_mailcopy(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_maildelete(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_mailedit(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_mailexport(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_mailfilter(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_mailforward(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_mailimport(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_mailinfo(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_mailmove(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_mailread(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_mailreply(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_mailsend(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_mailsendall(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_mailstatus(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_mailupdate(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_mailwrite(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_newmailfile(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_quit(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_readclose(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_readinfo(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_readprint(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_readsave(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_request(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_requestfile(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_requestfolder(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_requeststring(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_restart(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_screentoback(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_screentofront(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_setflag(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_setfolder(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_setmail(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_setmailfile(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_show(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_userinfo(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_writeattach(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_writebcc(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_writecc(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_writeeditor(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_writefrom(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_writeidentity(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_writeletter(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_writemailto(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_writeoptions(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_writequeue(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_writereplyto(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_writesend(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_writesubject(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);
void rx_writeto(struct RexxHost *, struct RexxParams *, enum RexxAction, struct RexxMsg *);

#endif /* REXX_H */
