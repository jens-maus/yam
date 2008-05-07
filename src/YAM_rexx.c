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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dos/dos.h>
#include <dos/rdargs.h>
#include <exec/memory.h>
#include <rexx/storage.h>
#include <rexx/rxslib.h>

#include <clib/alib_protos.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/rexxsyslib.h>

#include "extrasrc.h"

#include "YAM.h"
#include "YAM_rexx.h"
#include "YAM_utilities.h"

#include "Mime.h"

#include "Debug.h"

#define RexxPortBaseName "YAM"
#define RexxMsgExtension "YAM"

#if INCLUDE_VERSION >= 44
#define REXXMSG(msg) msg
#else
#define REXXMSG(msg) &msg->rm_Node
#endif

// not all SDKs do supply that new flag already
#ifndef RXFF_SCRIPT
#define RXFF_SCRIPT (1 << 21)
#endif

#if defined(__amigaos4__)
#define SetRexxVar(msg, var, val, len)  SetRexxVarFromMsg((var), (val), (msg))
#endif

struct rxs_stemnode
{
  struct rxs_stemnode *succ;
  char *name;
  char *value;
};

// flags for rxs_command->flags
#define ARB_CF_ENABLED     (1L << 0)

// flags for host->flags
#define ARB_HF_CMDSHELL    (1L << 0)
#define ARB_HF_USRMSGPORT  (1L << 1)

static void (*ARexxResultHook)(struct RexxHost *, struct RexxMsg *) = NULL;

/// rxs_commandlist[]
#define RESINDEX(stype) (((long)offsetof(struct stype,res)) / sizeof(long))

struct rxs_command rxs_commandlist[] =
{
  { "ADDRDELETE", "ALIAS", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_addrdelete, ARB_CF_ENABLED},
  { "ADDREDIT", "ALIAS,NAME,EMAIL,PGP,HOMEPAGE,STREET,CITY,COUNTRY,PHONE,COMMENT,BIRTHDATE/N,IMAGE,MEMBER/M,ADD/S", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_addredit, ARB_CF_ENABLED},
  { "ADDRFIND", "PATTERN/A,NAMEONLY/S,EMAILONLY/S", "ALIAS/M", RESINDEX(rxd_addrfind), (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_addrfind, ARB_CF_ENABLED},
  { "ADDRGOTO", "ALIAS/A", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_addrgoto, ARB_CF_ENABLED},
  { "ADDRINFO", "ALIAS/A", "TYPE,NAME,EMAIL,PGP,HOMEPAGE,STREET,CITY,COUNTRY,PHONE,COMMENT,BIRTHDATE/N,IMAGE,MEMBERS/M", RESINDEX(rxd_addrinfo), (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_addrinfo, ARB_CF_ENABLED},
  { "ADDRLOAD", "FILENAME/A", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_addrload, ARB_CF_ENABLED},
  { "ADDRNEW", "TYPE,ALIAS,NAME,EMAIL", "ALIAS", RESINDEX(rxd_addrnew), (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_addrnew, ARB_CF_ENABLED},
  { "ADDRRESOLVE", "ALIAS/A", "RECPT", RESINDEX(rxd_addrresolve), (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_addrresolve, ARB_CF_ENABLED},
  { "ADDRSAVE", "FILENAME", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_addrsave, ARB_CF_ENABLED},
  { "APPBUSY", "TEXT", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_appbusy, ARB_CF_ENABLED},
  { "APPNOBUSY", NULL, NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_appnobusy, ARB_CF_ENABLED},
  { "FOLDERINFO", "FOLDER", "NUMBER/N,NAME,PATH,TOTAL/N,NEW/N,UNREAD/N,SIZE/N,TYPE/N", RESINDEX(rxd_folderinfo), (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_folderinfo, ARB_CF_ENABLED},
  { "GETCONFIGINFO", "ITEM/A", "VALUE", RESINDEX(rxd_getconfiginfo), (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_getconfiginfo, ARB_CF_ENABLED},
  { "GETFOLDERINFO", "ITEM/A", "VALUE", RESINDEX(rxd_getfolderinfo), (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_getfolderinfo, ARB_CF_ENABLED},
  { "GETMAILINFO", "ITEM/A", "VALUE", RESINDEX(rxd_getmailinfo), (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_getmailinfo, ARB_CF_ENABLED},
  { "GETSELECTED", NULL, "NUM/N/M", RESINDEX(rxd_getselected), (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_getselected, ARB_CF_ENABLED},
  { "GETURL", "URL/A,FILENAME/A", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_geturl, ARB_CF_ENABLED},
  { "HELP", "FILE", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_help, ARB_CF_ENABLED},
  { "HIDE", NULL, NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_hide, ARB_CF_ENABLED},
  { "INFO", "ITEM/A", "VALUE", RESINDEX(rxd_info), (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_info, ARB_CF_ENABLED},
  { "ISONLINE", NULL, NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_isonline, ARB_CF_ENABLED},
  { "LISTSELECT", "MODE/A", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_listselect, ARB_CF_ENABLED},
  { "MAILARCHIVE", "FOLDER/A", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_mailarchive, ARB_CF_ENABLED},
  { "MAILBOUNCE", "QUIET/S", "WINDOW/N", RESINDEX(rxd_mailbounce), (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_mailbounce, ARB_CF_ENABLED},
  { "MAILCHANGESUBJECT", "SUBJECT/A", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_mailchangesubject, ARB_CF_ENABLED},
  { "MAILCHECK", "POP/K/N,MANUAL/S", "DOWNLOADED/N,ONSERVER/N,DUPSKIPPED/N,DELETED/N", RESINDEX(rxd_mailcheck), (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_mailcheck, ARB_CF_ENABLED},
  { "MAILCOPY", "FOLDER/A", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_mailcopy, ARB_CF_ENABLED},
  { "MAILDELETE", "ATONCE/S,FORCE/S", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_maildelete, ARB_CF_ENABLED},
  { "MAILEDIT", "QUIET/S", "WINDOW/N", RESINDEX(rxd_mailedit), (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_mailedit, ARB_CF_ENABLED},
  { "MAILEXPORT", "FILENAME/A,ALL/S,APPEND/S", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_mailexport, ARB_CF_ENABLED},
  { "MAILFILTER", "ALL/S", "CHECKED/N,BOUNCED/N,FORWARDED/N,REPLIED/N,EXECUTED/N,MOVED/N,DELETED/N", RESINDEX(rxd_mailfilter), (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_mailfilter, ARB_CF_ENABLED},
  { "MAILFORWARD", "QUIET/S", "WINDOW/N", RESINDEX(rxd_mailforward), (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_mailforward, ARB_CF_ENABLED},
  { "MAILIMPORT", "FILENAME/A,WAIT/S", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_mailimport, ARB_CF_ENABLED},
  { "MAILINFO", "INDEX/N", "INDEX/N,STATUS,FROM,TO,REPLYTO,SUBJECT,FILENAME,SIZE/N,DATE,FLAGS,MSGID", RESINDEX(rxd_mailinfo), (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_mailinfo, ARB_CF_ENABLED},
  { "MAILMOVE", "FOLDER/A", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_mailmove, ARB_CF_ENABLED},
  { "MAILREAD", "WINDOW/N,QUIET/S", "WINDOW/N", RESINDEX(rxd_mailread), (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_mailread, ARB_CF_ENABLED},
  { "MAILREPLY", "QUIET/S", "WINDOW/N", RESINDEX(rxd_mailreply), (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_mailreply, ARB_CF_ENABLED},
  { "MAILSEND", "ALL/S", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_mailsend, ARB_CF_ENABLED},
  { "MAILSENDALL", NULL, NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_mailsendall, ARB_CF_ENABLED},
  { "MAILSTATUS", "STATUS/A", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_mailstatus, ARB_CF_ENABLED},
  { "MAILUPDATE", NULL, NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_mailupdate, ARB_CF_ENABLED},
  { "MAILWRITE", "WINDOW/N,QUIET/S", "WINDOW/N", RESINDEX(rxd_mailwrite), (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_mailwrite, ARB_CF_ENABLED},
  { "NEWMAILFILE", "FOLDER", "FILENAME", RESINDEX(rxd_newmailfile), (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_newmailfile, ARB_CF_ENABLED},
  { "QUIT", "FORCE/S", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_quit, ARB_CF_ENABLED},
  { "READCLOSE", NULL, NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_readclose, ARB_CF_ENABLED},
  { "READINFO", NULL, "FILENAME/M,FILETYPE/M,FILESIZE/N/M,TEMPFILE/M", RESINDEX(rxd_readinfo), (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_readinfo, ARB_CF_ENABLED},
  { "READPRINT", "PART/N", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_readprint, ARB_CF_ENABLED},
  { "READSAVE", "PART/N,FILENAME/K,OVERWRITE/S", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_readsave, ARB_CF_ENABLED},
  { "REQUEST", "BODY/A,GADGETS/A", "RESULT/N", RESINDEX(rxd_request), (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_request, ARB_CF_ENABLED},
  { "REQUESTFOLDER", "BODY/A,EXCLUDEACTIVE/S", "FOLDER", RESINDEX(rxd_requestfolder), (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_requestfolder, ARB_CF_ENABLED},
  { "REQUESTSTRING", "BODY/A,STRING/K,SECRET/S", "STRING", RESINDEX(rxd_requeststring), (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_requeststring, ARB_CF_ENABLED},
  { "SCREENTOBACK", NULL, NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_screentoback, ARB_CF_ENABLED},
  { "SCREENTOFRONT", NULL, NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_screentofront, ARB_CF_ENABLED},
  { "SETFLAG", "VOL/K/N,PER/K/N", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_setflag, ARB_CF_ENABLED},
  { "SETFOLDER", "FOLDER/A", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_setfolder, ARB_CF_ENABLED},
  { "SETMAIL", "NUM/N/A", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_setmail, ARB_CF_ENABLED},
  { "SETMAILFILE", "MAILFILE/A", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_setmailfile, ARB_CF_ENABLED},
  { "SHOW", NULL, NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_show, ARB_CF_ENABLED},
  { "USERINFO", NULL, "USERNAME,EMAIL,REALNAME,CONFIG,MAILDIR,FOLDERS/N", RESINDEX(rxd_userinfo), (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_userinfo, ARB_CF_ENABLED},
  { "WRITEATTACH", "FILE/A,DESC,ENCMODE,CTYPE", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_writeattach, ARB_CF_ENABLED},
  { "WRITEBCC", "ADDRESS/A/M,ADD/S", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_writebcc, ARB_CF_ENABLED},
  { "WRITECC", "ADDRESS/A/M,ADD/S", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_writecc, ARB_CF_ENABLED},
  { "WRITEEDITOR", "COMMAND/A", "RESULT", RESINDEX(rxd_writeeditor), (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_writeeditor, ARB_CF_ENABLED},
  { "WRITEFROM", "ADDRESS/A", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_writefrom, ARB_CF_ENABLED},
  { "WRITELETTER", "FILE/A,NOSIG/S", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_writeletter, ARB_CF_ENABLED},
  { "WRITEMAILTO", "ADDRESS/A/M", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_writemailto, ARB_CF_ENABLED},
  { "WRITEOPTIONS", "DELETE/S,RECEIPT/S,NOTIF/S,ADDINFO/S,IMPORTANCE/N,SIG/N,SECURITY/N", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_writeoptions, ARB_CF_ENABLED},
  { "WRITEQUEUE", "HOLD/S", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_writequeue, ARB_CF_ENABLED},
  { "WRITEREPLYTO", "ADDRESS/A", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_writereplyto, ARB_CF_ENABLED},
  { "WRITESEND", NULL, NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_writesend, ARB_CF_ENABLED},
  { "WRITESUBJECT", "SUBJECT/A", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_writesubject, ARB_CF_ENABLED},
  { "WRITETO", "ADDRESS/A/M,ADD/S", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_writeto, ARB_CF_ENABLED},
};

///

/// ReplyRexxCommand
void ReplyRexxCommand(struct RexxMsg *rexxmessage, long primary, long secondary, char *result)
{
  ENTER();

  if(isFlagSet(rexxmessage->rm_Action, RXFF_RESULT))
  {
    if(primary == 0)
    {
      secondary = result
         ? (long)CreateArgstring(result, (ULONG)strlen(result))
         : (long)NULL;
    }
    else if(primary > 0)
    {
      char buf[16];

      snprintf(buf, sizeof(buf), "%ld", secondary);
      result = buf;
    }
    else
    {
       primary = -primary;
       result = (char *) secondary;
    }

    SetRexxVar( REXXMSG(rexxmessage), (STRPTR)"RC2", result, (LONG)strlen(result) );

    secondary = 0;
  }
  else if(primary < 0)
    primary = -primary;

  rexxmessage->rm_Result1 = primary;
  rexxmessage->rm_Result2 = secondary;
  ReplyMsg((struct Message *) rexxmessage);

  LEAVE();
}

///
/// FreeRexxCommand
void FreeRexxCommand(struct RexxMsg *rexxmessage)
{
  ENTER();

  if(rexxmessage->rm_Result1 == 0 && rexxmessage->rm_Result2 != 0)
    DeleteArgstring((APTR)rexxmessage->rm_Result2);

  if(rexxmessage->rm_Stdin != (BPTR)NULL && rexxmessage->rm_Stdin != Input())
    Close(rexxmessage->rm_Stdin);

  if(rexxmessage->rm_Stdout != (BPTR)NULL && rexxmessage->rm_Stdout != rexxmessage->rm_Stdin && rexxmessage->rm_Stdout != Output())
    Close(rexxmessage->rm_Stdout);

  DeleteArgstring((APTR)ARG0(rexxmessage));
  DeleteRexxMsg(rexxmessage);

  LEAVE();
}

///
/// CreateRexxCommand
static struct RexxMsg *CreateRexxCommand(struct RexxHost *host, char *buff, BPTR fh, int addFlags)
{
  struct RexxMsg *rexx_command_message;

  ENTER();

  if((rexx_command_message = CreateRexxMsg(host->port, RexxMsgExtension, host->port->mp_Node.ln_Name)) != NULL)
  {
    if((rexx_command_message->rm_Args[0] = (APTR)CreateArgstring(buff, strlen(buff))) != NULL)
    {
      rexx_command_message->rm_Action = RXCOMM | RXFF_RESULT | addFlags;
      rexx_command_message->rm_Stdin  = fh;
      rexx_command_message->rm_Stdout = fh;
    }
    else
    {
      DeleteRexxMsg(rexx_command_message);
      rexx_command_message = NULL;
    }
  }

  RETURN(rexx_command_message);
  return rexx_command_message;
}

///
/// CommandToRexx
static struct RexxMsg *CommandToRexx( struct RexxHost *host, struct RexxMsg *rexx_command_message )
{
  struct MsgPort *rexxport;
  struct RexxMsg *result = NULL;

  ENTER();

  Forbid();

  if((rexxport = FindPort(RXSDIR)) != NULL)
  {
    PutMsg(rexxport, &rexx_command_message->rm_Node);
    host->replies++;
    result = rexx_command_message;
  }

  Permit();

  RETURN(result);
  return result;
}

///
/// SendRexxCommand
struct RexxMsg *SendRexxCommand(struct RexxHost *host, char *buff, BPTR fh)
{
  struct RexxMsg *result = NULL;
  struct RexxMsg *rcm;

  ENTER();

  D(DBF_REXX, "executing ARexx script: '%s'", buff);

  // only RexxSysBase v45+ seems to support properly quoted
  // strings via the new RXFF_SCRIPT flag
  if(((struct Library *)RexxSysBase)->lib_Version >= 45)
    rcm = CreateRexxCommand(host, buff, fh, RXFF_SCRIPT);
  else
    rcm = CreateRexxCommand(host, buff, fh, 0);

  if(rcm != NULL)
    result =  CommandToRexx(host, rcm);

  RETURN(result);
  return result;
}

///
/// CloseDownARexxHost
void CloseDownARexxHost(struct RexxHost *host)
{
  ENTER();

  if(host->port != NULL)
  {
    struct RexxMsg *rexxmsg;

    // remove the port from the public list
    RemPort(host->port);

    // remove outstanding messages
    while(host->replies > 0)
    {
      WaitPort(host->port);

      while((rexxmsg = (struct RexxMsg *)GetMsg(host->port)) != NULL)
      {
        if(rexxmsg->rm_Node.mn_Node.ln_Type == NT_REPLYMSG)
        {
          if(rexxmsg->rm_Args[15] == NULL)
          {
            // it was a reply to a SendRexxCommand() call
            if(ARexxResultHook != NULL)
               ARexxResultHook(host, rexxmsg);
          }

          FreeRexxCommand(rexxmsg);
          host->replies--;
        }
        else
          ReplyRexxCommand(rexxmsg, -20, (long)"Host closing down", NULL);
      }
    }

    // empty the message port
    while((rexxmsg = (struct RexxMsg *)GetMsg(host->port)) != NULL)
      ReplyRexxCommand(rexxmsg, -20, (long)"Host closing down", NULL);

    if(isFlagClear(host->flags, ARB_HF_USRMSGPORT))
    {
      FreeSysObject(ASOT_PORT, host->port);
      host->port = NULL;
    }
  }

  if(host->rdargs != NULL)
  {
    FreeDosObject(DOS_RDARGS, host->rdargs);
    host->rdargs = NULL;
  }

  FreeVecPooled(G->SharedMemPool, host);

  LEAVE();
}

///
/// SetupARexxHost
struct RexxHost *SetupARexxHost(const char *basename, struct MsgPort *usrport)
{
  BOOL success = FALSE;
  struct RexxHost *host;

  ENTER();

  if(basename == NULL || basename[0] == '\0' )
    basename = RexxPortBaseName;

  if((host = AllocVecPooled(G->SharedMemPool, sizeof(struct RexxHost))) != NULL)
  {
    strlcpy(host->portname, basename, sizeof(host->portname));

    if((host->port = usrport) != NULL)
    {
      SET_FLAG(host->flags, ARB_HF_USRMSGPORT);
    }
    else if((host->port = AllocSysObjectTags(ASOT_PORT, TAG_DONE)) != NULL)
    {
      host->port->mp_Node.ln_Pri = 0;
    }

    if(host->port != NULL)
    {
      if((host->rdargs = AllocDosObject(DOS_RDARGS, NULL)) != NULL)
      {
        int ext = 0;

        host->rdargs->RDA_Flags = RDAF_NOPROMPT;

        Forbid();

        // create a unique name for the port
        while(FindPort(host->portname) != NULL)
          snprintf(host->portname, sizeof(host->portname), "%s.%d", basename, ++ext);

        host->portnumber = ext;
        host->port->mp_Node.ln_Name = host->portname;
        AddPort(host->port);

        Permit();

        success = TRUE;
      }
    }

    if(success == FALSE)
    {
      // something went wrong
      if(host->rdargs != NULL)
        FreeDosObject(DOS_RDARGS, host->rdargs);

      if(isFlagClear(host->flags, ARB_HF_USRMSGPORT))
        FreeSysObject(ASOT_PORT, host->port);

      FreeVecPooled(G->SharedMemPool, host);
      host = NULL;
    }
  }

  RETURN(host);
  return(host);
}

///
/// compare_rxs_commands
static int compare_rxs_commands(const void *key, const void *value)
{
  struct rxs_command *rxkey = (struct rxs_command *)key;
  struct rxs_command *rxvalue = (struct rxs_command *)value;

  return stricmp(rxkey->command, rxvalue->command);
}

///
/// ParseRXCommand
static struct rxs_command *ParseRXCommand(char **arg)
{
  char com[256], *s, *t;
  struct rxs_command key;
  struct rxs_command *cmd;

  ENTER();

  s = *arg;
  t = com;

  while(*s != '\0' && *s != ' ' && *s != '\n' && (unsigned int)(t - com + 1) < sizeof(com))
    *t++ = *s++;

  *t = '\0';
  while(*s == ' ')
    s++;
  *arg = s;

  SHOWSTRING(DBF_REXX, com);
  SHOWSTRING(DBF_REXX, *arg);

  key.command = com;
  cmd = (struct rxs_command *)bsearch(&key, rxs_commandlist, ARRAY_SIZE(rxs_commandlist), sizeof(struct rxs_command), compare_rxs_commands);

  RETURN(cmd);
  return cmd;
}

///
/// CreateVAR
static char *CreateVAR(struct rxs_stemnode *stem)
{
  char *var;
  struct rxs_stemnode *s;
  long size = 0;

  ENTER();

  if(stem == NULL || stem == (struct rxs_stemnode *)-1L)
  {
    RETURN((char *)stem);
    return (char *)stem;
  }

  for(s = stem; s; s = s->succ)
    size += strlen(s->value) + 1;

  // one byte more for the trailing NUL byte
  size++;

  if((var = AllocVecPooled(G->SharedMemPool, size)) == NULL)
  {
    RETURN((char *)-1);
    return (char *)-1;
  }

  *var = '\0';

  for(s = stem; s; s = s->succ)
  {
    strlcat(var, s->value, size);
    if(s->succ != NULL)
       strlcat(var, " ", size);
  }

  RETURN(var);
  return var;
}

///
/// new_stemnode
static struct rxs_stemnode *new_stemnode(struct rxs_stemnode **first, struct rxs_stemnode **oldNode)
{
  struct rxs_stemnode *newNode;

  ENTER();

  if((newNode = AllocVecPooled(G->SharedMemPool, sizeof(struct rxs_stemnode))) != NULL)
  {
    if(*oldNode != NULL)
    {
      (*oldNode)->succ = newNode;
      *oldNode = newNode;
    }
    else
    {
      *first = newNode;
      *oldNode = newNode;
    }
  }

  RETURN(newNode);
  return newNode;
}

///
/// free_stemlist
static void free_stemlist(struct rxs_stemnode *first )
{
  ENTER();

  if((long)first != -1)
  {
    struct rxs_stemnode *next;

    for( ; first != NULL; first = next)
    {
      next = first->succ;

      if(first->name != NULL)
        free(first->name);

      if(first->value != NULL)
        free(first->value);

      FreeVecPooled(G->SharedMemPool, first);
    }
  }

  LEAVE();
}

///
/// CreateSTEM
static struct rxs_stemnode *CreateSTEM( struct rxs_command *rxc, LONG *resarray, char *stembase )
{
   struct rxs_stemnode *first = NULL, *old = NULL, *new;
   char resb[512];
   char *rb;
   char longbuff[16];
   const char *rs;

   rb = resb;
   if( stembase )
   {
      while( *stembase )
      {
         *rb++ = toupper(*stembase);
         stembase++;
      }
   }
   *rb = '\0';

   rb = resb + strlen(resb);
   rs = rxc->results;

   while( *rs )
   {
      char *t = rb;
      BOOL optn = FALSE, optm = FALSE;

      while( *rs && *rs != ',' )
      {
         if( *rs == '/' )
         {
            ++rs;
            if( *rs == 'N' ) optn = TRUE;
            else if( *rs == 'M' ) optm = TRUE;
         }
         else
            *t++ = *rs;

         ++rs;
      }

      if( *rs == ',' ) ++rs;
      *t = '\0';

      // create the results
      if( !*resarray )
      {
         ++resarray;
         continue;
      }

      if( optm )
      {
         long *r, index = 0;
         LONG **subarray = (LONG **) *resarray++;
         struct rxs_stemnode *countnd;

         // number of elements
         if( !(new = new_stemnode(&first, &old)) )
         {
            free_stemlist( first );
            return( (struct rxs_stemnode *) -1L );
         }
         countnd = new;

         // the elements
         while((r = *subarray++))
         {
            if( !(new = new_stemnode(&first, &old)) )
            {
               free_stemlist( first );
               return( (struct rxs_stemnode *) -1L );
            }

            snprintf(t, sizeof(resb)-(t-resb), ".%ld", index++);
            new->name = strdup(resb);

            if( optn )
            {
               snprintf(longbuff, sizeof(longbuff), "%ld", *r);
               new->value = strdup(longbuff);
            }
            else
            {
               new->value = strdup((char *)r);
            }
         }

         // the count node
         strlcpy(t, ".COUNT", sizeof(resb)-(t-resb));
         countnd->name = strdup(resb);

         snprintf(longbuff, sizeof(longbuff), "%ld", index);
         countnd->value = strdup(longbuff);
      }
      else
      {
         // create a new node
         if( !(new = new_stemnode(&first, &old)) )
         {
            free_stemlist( first );
            return( (struct rxs_stemnode *) -1L );
         }

         new->name = strdup(resb);

         if( optn )
         {
            snprintf(longbuff, sizeof(longbuff), "%ld", *((long *)*resarray));
            new->value = strdup(longbuff);
            ++resarray;
         }
         else
         {
            new->value = strdup((char *)(*resarray++));
         }
      }
   }

   return( first );
}

///
/// DoRXCommand
void DoRXCommand( struct RexxHost *host, struct RexxMsg *rexxmsg )
{
   struct rxs_command *rxc = 0;
   char *argb, *arg;

   LONG *array = NULL;
   LONG *argarray;
   LONG *resarray;

   char *cargstr = NULL;
   long rc=20, rc2;
   char *result = NULL;

   ENTER();

   if((argb = AllocVecPooled(G->SharedMemPool, (ULONG)strlen((char *) ARG0(rexxmsg)) + 2)) == NULL)
   {
     rc2 = ERROR_NO_FREE_STORE;
     goto drc_cleanup;
   }

   // which command
   snprintf(argb, strlen((char *)ARG0(rexxmsg))+2, "%s\n", ARG0(rexxmsg));
   arg = argb;

   SHOWSTRING(DBF_REXX, arg);

   if(!(rxc = ParseRXCommand( &arg )))
   {
      // send messahe to ARexx, perhaps a script exists
      struct RexxMsg *rm;

      if((rm = CreateRexxCommand(host, (char *) ARG0(rexxmsg), 0, 0)))
      {
         /* Original-Msg merken */
         rm->rm_Args[15] = (STRPTR) rexxmsg;

         if( CommandToRexx(host, rm) )
         {
            // the reply is done later by the dispatcher
            if(argb)
            {
              FreeVecPooled(G->SharedMemPool, argb);
            }

            LEAVE();
            return;
         }
         else
            rc2 = ERROR_NOT_IMPLEMENTED;
      }
      else
         rc2 = ERROR_NO_FREE_STORE;

      goto drc_cleanup;
   }

   if(isFlagClear(rxc->flags, ARB_CF_ENABLED))
   {
      rc = -10;
      rc2 = (long)"Command disabled";
      goto drc_cleanup;
   }

   // get memory for the arguments
   (rxc->function)(host, (void **)(APTR)&array, RXIF_INIT, rexxmsg);

   cargstr = AllocVecPooled(G->SharedMemPool, (ULONG)(rxc->args ? 15+strlen(rxc->args) : 15));

   if(!array || !cargstr)
   {
      rc2 = ERROR_NO_FREE_STORE;
      goto drc_cleanup;
   }

   argarray = array + 2;
   resarray = array + rxc->resindex;

   // parse the arguments
   if( rxc->results )
      strlcpy(cargstr, "VAR/K,STEM/K", (ULONG)(rxc->args ? 15+strlen(rxc->args) : 15));
   else
      *cargstr = '\0';

   if( rxc->args )
   {
      if(*cargstr)
         strlcat(cargstr, ",", (ULONG)(rxc->args ? 15+strlen(rxc->args) : 15));

      strlcat(cargstr, rxc->args, (ULONG)(rxc->args ? 15+strlen(rxc->args) : 15));
   }

   if( *cargstr )
   {
      host->rdargs->RDA_Source.CS_Buffer = arg;
      host->rdargs->RDA_Source.CS_Length = strlen(arg);
      host->rdargs->RDA_Source.CS_CurChr = 0;
      host->rdargs->RDA_DAList = 0;
      host->rdargs->RDA_Buffer = NULL;
      host->rdargs->RDA_BufSiz = 0;
      host->rdargs->RDA_ExtHelp = NULL;
      host->rdargs->RDA_Flags = RDAF_NOPROMPT;

      if(ReadArgs(cargstr, argarray, host->rdargs) == NULL)
      {
         rc = 10;
         rc2 = IoErr();
         goto drc_cleanup;
      }
   }

   // call the function
   (rxc->function)( host, (void **)(APTR)&array, RXIF_ACTION, rexxmsg );

   rc = array[0];
   rc2 = array[1];

   // evaluate the results
   if( rxc->results && rc==0 &&
      (rexxmsg->rm_Action & RXFF_RESULT) )
   {
      struct rxs_stemnode *stem, *s;

      stem = CreateSTEM( rxc, resarray, (char *)argarray[1] );
      result = CreateVAR( stem );

      if( result )
      {
         if( argarray[0] )
         {
            // VAR
            if( (long) result == -1 )
            {
               rc = 20;
               rc2 = ERROR_NO_FREE_STORE;
            }
            else
            {
               char *rb;

               for( rb = (char *) argarray[0]; *rb; ++rb )
                  *rb = toupper( *rb );

               if( SetRexxVar( REXXMSG(rexxmsg),
                  (STRPTR)(*((char *)argarray[0]) ? (char *)argarray[0] : "RESULT"),
                  result, (LONG)strlen(result) ) )
               {
                  rc = -10;
                  rc2 = (long) "Unable to set Rexx variable";
               }

               FreeVecPooled(G->SharedMemPool, result);
            }

            result = NULL;
         }

         if( !rc && argarray[1] )
         {
            // STEM
            if( (long) stem == -1 )
            {
               rc = 20;
               rc2 = ERROR_NO_FREE_STORE;
            }
            else
            {
               for( s = stem; s; s = s->succ )
                  rc |= SetRexxVar( REXXMSG(rexxmsg), s->name, s->value, (LONG)strlen(s->value) );

               if( rc )
               {
                  rc = -10;
                  rc2 = (long) "Unable to set Rexx variable";
               }

               if(result && (long)result != -1)
               {
                 FreeVecPooled(G->SharedMemPool, result);
               }
            }

            result = NULL;
         }

         // is a normal result possible?
         if( (long) result == -1 )
         {
            // no!
            rc = 20;
            rc2 = ERROR_NO_FREE_STORE;
            result = NULL;
         }
      }

      free_stemlist( stem );
   }

drc_cleanup:

   // return RESULT only, if neither VAR nor STEM
   ReplyRexxCommand(rexxmsg, rc, rc2, result);

   // free the memory
   if(result != NULL)
     FreeVecPooled(G->SharedMemPool, result);

   FreeArgs(host->rdargs);

   if(cargstr != NULL)
     FreeVecPooled(G->SharedMemPool, cargstr);

   if(array != NULL)
     (rxc->function)(host, (void **)(APTR)&array, RXIF_FREE, rexxmsg);

   if(argb != NULL)
     FreeVecPooled(G->SharedMemPool, argb);

   LEAVE();
}

///
/// ARexxDispatch
void ARexxDispatch( struct RexxHost *host )
{
  struct RexxMsg *rexxmsg;

  ENTER();

  while((rexxmsg = (struct RexxMsg *)GetMsg(host->port)) != NULL)
  {
    if((rexxmsg->rm_Action & RXCODEMASK) != RXCOMM)
    {
      // No Rexx-Message
      ReplyMsg( (struct Message *) rexxmsg );
    }
    else if(rexxmsg->rm_Node.mn_Node.ln_Type == NT_REPLYMSG)
    {
      struct RexxMsg *org;

      if((org = (struct RexxMsg *)rexxmsg->rm_Args[15]) != NULL)
       {
         // Reply to a forwarded Msg
         if(rexxmsg->rm_Result1 != 0)
         {
           // command unknown
           ReplyRexxCommand(org, 20, ERROR_NOT_IMPLEMENTED, NULL);
         }
         else
         {
           ReplyRexxCommand(org, 0, 0, (char *)rexxmsg->rm_Result2);
         }
       }
       else
       {
         // reply to a SendRexxCommand()-Call
         if(ARexxResultHook != NULL)
            ARexxResultHook(host, rexxmsg);
       }

       FreeRexxCommand(rexxmsg);
       host->replies--;
    }
    else if(ARG0(rexxmsg) != NULL)
    {
      DoRXCommand(host, rexxmsg);
    }
    else
    {
      ReplyMsg((struct Message *)rexxmsg);
    }
  }

  LEAVE();
}
///
