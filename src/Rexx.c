/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2013 YAM Open Source Team

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
#include "YAM_error.h"
#include "YAM_utilities.h"

#include "Locale.h"
#include "Rexx.h"

#include "Debug.h"

#define RexxPortBaseName "YAM"
#define RexxMsgExtension "YAM"

#if INCLUDE_VERSION >= 44
#define REXXMSG(msg) (struct RexxMsg *)msg
#else
#define REXXMSG(msg) (struct RexxMsg *)&msg->rm_Node
#endif

// not all SDKs do supply that new flag already
#ifndef RXFF_SCRIPT
#define RXFF_SCRIPT (1 << 21)
#endif

#if defined(__amigaos4__)
#define SetRexxVar(msg, var, val, len)  SetRexxVarFromMsg((var), (val), (msg))
#endif

struct StemNode
{
  struct MinNode node;
  char *name;
  char *value;
};

// flags for host->flags
#define ARB_HF_CMDSHELL    (1L << 0)
#define ARB_HF_USRMSGPORT  (1L << 1)

static void (*ARexxResultHook)(struct RexxHost *, struct RexxMsg *) = NULL;

/// rxs_commandlist[]
// !! keep this list sorted alphabetically, we are performing a binary search on it !!
struct rxs_command rxs_commandlist[] =
{
  { "ADDRDELETE", "ALIAS", NULL, rx_addrdelete },
  { "ADDREDIT", "ALIAS,NAME,EMAIL,PGP,HOMEPAGE,STREET,CITY,COUNTRY,PHONE,COMMENT,BIRTHDATE/N,IMAGE,MEMBER/M,ADD/S", NULL, rx_addredit },
  { "ADDRFIND", "PATTERN/A,NAMEONLY/S,EMAILONLY/S", "ALIAS/M", rx_addrfind },
  { "ADDRGOTO", "ALIAS/A", NULL, rx_addrgoto },
  { "ADDRINFO", "ALIAS/A", "TYPE,NAME,EMAIL,PGP,HOMEPAGE,STREET,CITY,COUNTRY,PHONE,COMMENT,BIRTHDATE/N,IMAGE,MEMBERS/M", rx_addrinfo },
  { "ADDRLOAD", "FILENAME/A,OPEN/S", NULL, rx_addrload },
  { "ADDRNEW", "TYPE,ALIAS,NAME,EMAIL", "ALIAS", rx_addrnew },
  { "ADDRRESOLVE", "ALIAS/A", "RECPT", rx_addrresolve },
  { "ADDRSAVE", "FILENAME", NULL, rx_addrsave },
  { "APPBUSY", "TEXT", NULL, rx_appbusy },
  { "APPNOBUSY", NULL, NULL, rx_appnobusy },
  { "FINDMAIL", "MSGID/A", "FOLDER", rx_findmail },
  { "FLUSHINDEXES", NULL, NULL, rx_flushindexes },
  { "FOLDERINFO", "FOLDER", "NUMBER/N,NAME,PATH,TOTAL/N,NEW/N,UNREAD/N,SIZE/N,TYPE/N", rx_folderinfo },
  { "GETCONFIGINFO", "ITEM/A,IDENTITY/K/N", "VALUE", rx_getconfiginfo },
  { "GETFOLDERINFO", "ITEM/A", "VALUE", rx_getfolderinfo },
  { "GETMAILINFO", "ITEM/A", "VALUE", rx_getmailinfo },
  { "GETSELECTED", NULL, "NUM/N/M", rx_getselected },
  { "GETURL", "URL/A,FILENAME/A", NULL, rx_geturl },
  { "HELP", "FILE", NULL, rx_help },
  { "HIDE", NULL, NULL, rx_hide },
  { "INFO", "ITEM/A", "VALUE", rx_info },
  { "ISONLINE", NULL, NULL, rx_isonline },
  { "LISTFREEZE", "LIST/A", NULL, rx_listfreeze },
  { "LISTSELECT", "MODE/A", NULL, rx_listselect },
  { "LISTUNFREEZE", "LIST/A", NULL, rx_listunfreeze },
  { "MAILARCHIVE", "FOLDER/A", NULL, rx_mailarchive },
  { "MAILBOUNCE", "QUIET/S", "WINDOW/N", rx_mailbounce },
  { "MAILCHANGESUBJECT", "SUBJECT/A", NULL, rx_mailchangesubject },
  { "MAILCHECK", "POP/K/N,MANUAL/S", "DOWNLOADED/N,ONSERVER/N,DUPSKIPPED/N,DELETED/N", rx_mailcheck },
  { "MAILCOPY", "FOLDER/A", NULL, rx_mailcopy },
  { "MAILDELETE", "ATONCE/S,FORCE/S", NULL, rx_maildelete },
  { "MAILEDIT", "QUIET/S", "WINDOW/N", rx_mailedit },
  { "MAILEXPORT", "FILENAME/A,ALL/S,APPEND/S,QUIET/S", NULL, rx_mailexport },
  { "MAILFILTER", "ALL/S", "CHECKED/N,BOUNCED/N,FORWARDED/N,REPLIED/N,EXECUTED/N,MOVED/N,DELETED/N", rx_mailfilter },
  { "MAILFORWARD", "QUIET/S", "WINDOW/N", rx_mailforward },
  { "MAILIMPORT", "FILENAME/A,QUIET/S,WAIT/S", NULL, rx_mailimport },
  { "MAILINFO", "INDEX/N", "INDEX/N,STATUS,FROM,TO,REPLYTO,CC,BCC,RESENTTO,SUBJECT,FILENAME,SIZE/N,DATE,FLAGS,MSGID,FROMALL/M,TOALL/M,REPLYTOALL/M,CCALL/M,BCCALL/M,RESENTTOALL/M", rx_mailinfo },
  { "MAILMOVE", "FOLDER/A", NULL, rx_mailmove },
  { "MAILREAD", "WINDOW/N,QUIET/S", "WINDOW/N", rx_mailread },
  { "MAILREPLY", "QUIET/S", "WINDOW/N", rx_mailreply },
  { "MAILSEND", "ALL/S", NULL, rx_mailsend },
  { "MAILSENDALL", NULL, NULL, rx_mailsendall },
  { "MAILSTATUS", "STATUS/A", NULL, rx_mailstatus },
  { "MAILUPDATE", NULL, NULL, rx_mailupdate },
  { "MAILWRITE", "WINDOW/N,QUIET/S", "WINDOW/N", rx_mailwrite },
  { "NEWMAILFILE", "FOLDER", "FILENAME", rx_newmailfile },
  { "QUIT", "FORCE/S", NULL, rx_quit },
  { "READCLOSE", NULL, NULL, rx_readclose },
  { "READINFO", NULL, "FILENAME/M,FILETYPE/M,FILESIZE/N/M,TEMPFILE/M", rx_readinfo },
  { "READPRINT", "PART/N", NULL, rx_readprint },
  { "READSAVE", "PART/N,FILENAME/K,OVERWRITE/S", NULL, rx_readsave },
  { "REQUEST", "BODY/A,GADGETS/A", "RESULT/N", rx_request },
  { "REQUESTFILE", "TITLE/A,DRAWER,FILE,MULTISELECT/S,DRAWERSONLY/S,SAVEMODE/S", "DRAWER,FILES/M", rx_requestfile },
  { "REQUESTFOLDER", "BODY/A,EXCLUDEACTIVE/S", "FOLDER", rx_requestfolder },
  { "REQUESTSTRING", "BODY/A,STRING/K,SECRET/S", "STRING", rx_requeststring },
  { "RESTART", "FORCE/S", NULL, rx_restart },
  { "SCREENTOBACK", NULL, NULL, rx_screentoback },
  { "SCREENTOFRONT", NULL, NULL, rx_screentofront },
  { "SETFLAG", "VOL/K/N,PER/K/N", NULL, rx_setflag },
  { "SETFOLDER", "FOLDER/A", NULL, rx_setfolder },
  { "SETMAIL", "NUM/N,MSGID/K", NULL, rx_setmail },
  { "SETMAILFILE", "MAILFILE/A", NULL, rx_setmailfile },
  { "SHOW", NULL, NULL, rx_show },
  { "USERINFO", "IDENTITY/K/N", "USERNAME,EMAIL,REALNAME,CONFIG,MAILDIR,FOLDERS/N", rx_userinfo },
  { "WRITEATTACH", "FILE/A,DESC,ENCMODE,CTYPE", NULL, rx_writeattach },
  { "WRITEBCC", "ADDRESS/A/M,ADD/S", NULL, rx_writebcc },
  { "WRITECC", "ADDRESS/A/M,ADD/S", NULL, rx_writecc },
  { "WRITEEDITOR", "COMMAND/A", "RESULT", rx_writeeditor },
  { "WRITEFROM", "ADDRESS/A", NULL, rx_writefrom },
  { "WRITEIDENTITY", "IDENTITY/K,ADDRESS/K", NULL, rx_writeidentity },
  { "WRITELETTER", "FILE/A,NOSIG/S", NULL, rx_writeletter },
  { "WRITEMAILTO", "ADDRESS/A/M", NULL, rx_writemailto },
  { "WRITEOPTIONS", "DELETE/S,RECEIPT/S,NOTIF/S,ADDINFO/S,IMPORTANCE/N,SIG/N,SECURITY/N", NULL, rx_writeoptions },
  { "WRITEQUEUE", "DRAFT=HOLD/S", NULL, rx_writequeue },
  { "WRITEREPLYTO", "ADDRESS/A", NULL, rx_writereplyto },
  { "WRITESEND", NULL, NULL, rx_writesend },
  { "WRITESUBJECT", "SUBJECT/A", NULL, rx_writesubject },
  { "WRITETO", "ADDRESS/A/M,ADD/S", NULL, rx_writeto },
  { NULL, NULL, NULL, NULL }
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
    else
    {
      if(primary > 0)
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

      SetRexxVar(REXXMSG(rexxmessage), (STRPTR)"RC2", result, (LONG)strlen(result));

      secondary = 0;
    }
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

  if(rexxmessage->rm_Stdin != ZERO && rexxmessage->rm_Stdin != Input())
    Close(rexxmessage->rm_Stdin);

  if(rexxmessage->rm_Stdout != ZERO && rexxmessage->rm_Stdout != rexxmessage->rm_Stdin && rexxmessage->rm_Stdout != Output())
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

  if((rexx_command_message = CreateRexxMsg(host->port, (APTR)RexxMsgExtension, host->port->mp_Node.ln_Name)) != NULL)
  {
    if((rexx_command_message->rm_Args[0] = (APTR)CreateArgstring(buff, strlen(buff))) != 0)
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
static struct RexxMsg *CommandToRexx(struct RexxHost *host, struct RexxMsg *rexx_command_message)
{
  struct MsgPort *rexxport;
  struct RexxMsg *result = NULL;

  ENTER();

  Forbid();

  if((rexxport = FindPort((APTR)RXSDIR)) != NULL)
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

  #if defined(__MORPHOS__)
  // on MorphOS we have the special situation that per default there exists no
  // working rexxsyslib.library as the MorphOS developers have never implemented
  // it. Nevertheless they have chosen to install a fake/wrapper library which
  // comes with a v50 version and will only popup a warning requester which
  // gives the impression that the application doesn't have a properly implemented
  // ARexx implementation as it never states the word "MorphOS". Thus, we go and
  // identify this situation and warn the user ourselve to make that more clear.
  if(((struct Library *)(RexxSysBase))->lib_Version == 50 &&
     ((struct Library *)(RexxSysBase))->lib_Revision <= 4)
  {
    ER_NewError(tr(MSG_ER_MORPHOS_REXX_WARNING));
  }
  #endif

  D(DBF_REXX, "executing ARexx script: '%s'", buff);

  // only RexxSysBase v45+ seems to support properly quoted
  // strings via the new RXFF_SCRIPT flag
  if(LIB_VERSION_IS_AT_LEAST(RexxSysBase, 45, 0) == TRUE)
    rcm = CreateRexxCommand(host, buff, fh, RXFF_SCRIPT);
  else
    rcm = CreateRexxCommand(host, buff, fh, 0);

  if(rcm != NULL)
    result = CommandToRexx(host, rcm);

  D(DBF_REXX, "executedARexx script: '%s' => %08lx", buff, result);

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
          if(rexxmsg->rm_Args[15] == 0)
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

    if(usrport != NULL)
    {
      host->port = usrport;
      setFlag(host->flags, ARB_HF_USRMSGPORT);
    }
    else
    {
      host->port = AllocSysObjectTags(ASOT_PORT, TAG_DONE);
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
  cmd = (struct rxs_command *)bsearch(&key, rxs_commandlist, ARRAY_SIZE(rxs_commandlist)-1, sizeof(struct rxs_command), compare_rxs_commands);

  SHOWVALUE(DBF_REXX, cmd);

  RETURN(cmd);
  return cmd;
}

///
/// CreateVAR
static char *CreateVAR(struct MinList *stemList)
{
  char *var = NULL;

  ENTER();

  if(IsMinListEmpty(stemList) == FALSE)
  {
    long size = 0;
    struct StemNode *stemNode;

    // count the length of all variable names
    IterateList(stemList, struct StemNode *, stemNode)
    {
      size += strlen(stemNode->value) + 1;
    }

    // one byte more for the trailing NUL byte
    size++;

    if((var = AllocVecPooled(G->SharedMemPool, size)) != NULL)
    {
      var[0] = '\0';

      IterateList(stemList, struct StemNode *, stemNode)
      {
        // append an additional space except for the first variable
        if(var[0] != '\0')
           strlcat(var, " ", size);

        strlcat(var, stemNode->value, size);
      }
    }
  }

  RETURN(var);
  return var;
}

///
/// FreeStemList
static void FreeStemList(struct MinList *list)
{
  struct StemNode *node;

  ENTER();

  while((node = (struct StemNode *)RemHead((struct List *)list)) != NULL)
  {
    free(node->name);
    free(node->value);
    free(node);
  }

  LEAVE();
}

///
/// CreateSTEM
static BOOL CreateSTEM(struct MinList *stemList, struct rxs_command *rxc, LONG *resarray, char *stembase)
{
  char resb[512];
  char *rb;
  char longbuff[16];
  const char *rs;
  BOOL success = TRUE;

  ENTER();

  NewMinList(stemList);

  // create an upper case copy of the STEM name
  rb = resb;
  if(stembase != NULL)
  {
    while(*stembase != '\0')
    {
      *rb++ = toupper(*stembase);
      stembase++;
    }
  }
  *rb = '\0';

  rb = resb + strlen(resb);
  rs = rxc->results;

  while(*rs != '\0' && success == TRUE)
  {
    char *t = rb;
    BOOL isNumber = FALSE;
    BOOL isMulti = FALSE;

    // parse the variable names and check for numbers and multiple arguments
    while(*rs != '\0' && *rs != ',')
    {
      if(*rs == '/' )
      {
         rs++;
         if(*rs == 'N' )
           isNumber = TRUE;
         else if(*rs == 'M')
           isMulti = TRUE;
      }
      else
        *t++ = *rs;

      rs++;
    }

    if(*rs == ',' )
      rs++;

    *t = '\0';

    // create the results
    if(*resarray == 0)
    {
       resarray++;
       continue;
    }

    if(isMulti == TRUE)
    {
      struct StemNode *countNode;

      // number of elements
      if((countNode = malloc(sizeof(*countNode))) != NULL)
      {
        long *r;
        long stemIndex = 0;
        LONG **subarray = (LONG **)*resarray++;

        AddTail((struct List *)stemList, (struct Node *)countNode);

        // the elements
        while((r = (long *)*subarray++) != NULL && success == TRUE)
        {
          struct StemNode *stemNode;

          if((stemNode = malloc(sizeof(*stemNode))) != NULL)
          {
            snprintf(t, sizeof(resb)-(t-resb), ".%ld", stemIndex++);
            stemNode->name = strdup(resb);

            if(isNumber == TRUE)
            {
              snprintf(longbuff, sizeof(longbuff), "%ld", *r);
              stemNode->value = strdup(longbuff);
            }
            else
            {
              stemNode->value = strdup((char *)r);
            }

            AddTail((struct List *)stemList, (struct Node *)stemNode);
          }
          else
            success = FALSE;
        }

        // the count node
        strlcpy(t, ".COUNT", sizeof(resb)-(t-resb));
        countNode->name = strdup(resb);

        snprintf(longbuff, sizeof(longbuff), "%ld", stemIndex);
        countNode->value = strdup(longbuff);
      }
    }
    else
    {
      struct StemNode *stemNode;

      // create a new node
      if((stemNode = malloc(sizeof(*stemNode))) != NULL)
      {
        AddTail((struct List *)stemList, (struct Node *)stemNode);

        stemNode->name = strdup(resb);

        if(isNumber == TRUE)
        {
          snprintf(longbuff, sizeof(longbuff), "%ld", *((long *)*resarray));
          stemNode->value = strdup(longbuff);
        }
        else
        {
          stemNode->value = strdup((char *)*resarray);
        }

        resarray++;
      }
      else
        success = FALSE;
    }
  }

  RETURN(success);
  return success;
}

///
/// DoRXCommand
void DoRXCommand(struct RexxHost *host, struct RexxMsg *rexxmsg)
{
  struct rxs_command *rxc = NULL;
  char *argb, *arg;
  struct RexxParams params;
  ULONG carglen;
  char *cargstr = NULL;
  char *result = NULL;

  ENTER();

  // clear all values
  params.rc = 0;
  params.rc2 = 0;
  params.args = NULL;
  params.results = NULL;
  params.optional = NULL;

  if((argb = AllocVecPooled(G->SharedMemPool, (ULONG)strlen((char *)ARG0(rexxmsg)) + 2)) == NULL)
  {
    params.rc2 = ERROR_NO_FREE_STORE;
    goto drc_cleanup;
  }

  // which command
  snprintf(argb, strlen((char *)ARG0(rexxmsg))+2, "%s\n", ARG0(rexxmsg));
  arg = argb;

  SHOWSTRING(DBF_REXX, arg);

  if((rxc = ParseRXCommand(&arg)) == NULL)
  {
    // send message to ARexx, perhaps a script exists
    struct RexxMsg *rm;

    if((rm = CreateRexxCommand(host, (char *)ARG0(rexxmsg), 0, 0)) != NULL)
    {
      // remember original message
      rm->rm_Args[15] = (APTR)rexxmsg;

      if(CommandToRexx(host, rm) != NULL)
      {
        // the reply is done later by the dispatcher
        if(argb != NULL)
          FreeVecPooled(G->SharedMemPool, argb);

        LEAVE();
        return;
      }
      else
      {
        params.rc = 20;
        params.rc2 = ERROR_NOT_IMPLEMENTED;
      }
    }
    else
    {
      params.rc = 20;
      params.rc2 = ERROR_NO_FREE_STORE;
    }

    goto drc_cleanup;
  }

  D(DBF_REXX, "RXIF_INIT '%s'", rxc->command);
  // get memory for the arguments and the offset of a possible result array
  (rxc->function)(host, &params, RXIF_INIT, rexxmsg);
  if(params.rc != 0 || params.args == NULL || (rxc->results != NULL && params.results == NULL))
  {
    params.rc = 20;
    params.rc2 = ERROR_NO_FREE_STORE;
    goto drc_cleanup;
  }

  carglen = (rxc->args != NULL) ? 15 + strlen(rxc->args) : 15;
  cargstr = AllocVecPooled(G->SharedMemPool, carglen);
  if(cargstr == NULL)
  {
    params.rc = 20;
    params.rc2 = ERROR_NO_FREE_STORE;
    goto drc_cleanup;
  }

  // parse the arguments
  if(rxc->results != NULL)
    strlcpy(cargstr, "VAR/K,STEM/K", carglen);
  else
    cargstr[0] = '\0';

  if(rxc->args != NULL)
  {
    if(cargstr[0] != '\0')
      strlcat(cargstr, ",", carglen);

    strlcat(cargstr, rxc->args, carglen);
  }

  D(DBF_REXX, "using template '%s' to parse string '%s'", cargstr, arg);

  if(cargstr[0] != '\0')
  {
    host->rdargs->RDA_Source.CS_Buffer = arg;
    host->rdargs->RDA_Source.CS_Length = strlen(arg);
    host->rdargs->RDA_Source.CS_CurChr = 0;
    host->rdargs->RDA_DAList = 0;
    host->rdargs->RDA_Buffer = NULL;
    host->rdargs->RDA_BufSiz = 0;
    host->rdargs->RDA_ExtHelp = NULL;
    host->rdargs->RDA_Flags = RDAF_NOPROMPT;

    if(ReadArgs(cargstr, params.args, host->rdargs) == NULL)
    {
      params.rc = 10;
      params.rc2 = IoErr();
      goto drc_cleanup;
    }
  }

  // call the function
  D(DBF_REXX, "RXIF_ACTION '%s'", rxc->command);
  (rxc->function)(host, &params, RXIF_ACTION, rexxmsg);

  // evaluate the results
  if(rxc->results != NULL && params.rc == 0 && isFlagSet(rexxmsg->rm_Action, RXFF_RESULT))
  {
    struct RexxResult *varStem = (struct RexxResult *)params.args;
    struct MinList stemList;

    if(CreateSTEM(&stemList, rxc, params.results, varStem->stem) == TRUE)
    {
      if((result = CreateVAR(&stemList)) != NULL)
      {
        if(varStem->var != NULL)
        {
          // VAR
          char *rb;

          // convert to upper case
          for(rb = varStem->var; *rb; ++rb)
             *rb = toupper(*rb);

          if(SetRexxVar(REXXMSG(rexxmsg), (varStem->var[0] != '\0') ? varStem->var : (char *)"RESULT", result, strlen(result)) != 0)
          {
            params.rc = -10;
            params.rc2 = (long)"Unable to set Rexx variable";
          }

          FreeVecPooled(G->SharedMemPool, result);
          result = NULL;
        }

        if(params.rc == 0 && varStem->stem != NULL)
        {
          // STEM
          struct StemNode *stemNode;

          IterateList(&stemList, struct StemNode *, stemNode)
          {
            params.rc |= SetRexxVar(REXXMSG(rexxmsg), stemNode->name, stemNode->value, strlen(stemNode->value));
          }

          if(params.rc != 0)
          {
            params.rc = -10;
            params.rc2 = (long)"Unable to set Rexx variable";
          }

          if(result != NULL)
          {
            FreeVecPooled(G->SharedMemPool, result);
            result = NULL;
          }
        }
      }
      else
      {
        params.rc = 20;
        params.rc2 = ERROR_NO_FREE_STORE;
      }
    }
    else
    {
      params.rc = 20;
      params.rc2 = ERROR_NO_FREE_STORE;
    }

    FreeStemList(&stemList);
  }

drc_cleanup:

  // return RESULT only, if neither VAR nor STEM
  ReplyRexxCommand(rexxmsg, params.rc, params.rc2, result);

  // free the memory
  if(result != NULL)
    FreeVecPooled(G->SharedMemPool, result);

  FreeArgs(host->rdargs);

  if(cargstr != NULL)
    FreeVecPooled(G->SharedMemPool, cargstr);

  if(rxc != NULL)
  {
    D(DBF_REXX, "RXIF_FREE '%s'", rxc->command);
    (rxc->function)(host, &params, RXIF_FREE, rexxmsg);
  }

  if(argb != NULL)
    FreeVecPooled(G->SharedMemPool, argb);

  LEAVE();
}

///
/// ARexxDispatch
void ARexxDispatch(struct RexxHost *host)
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
        D(DBF_REXX, "received reply of a forwarded ARexx message");
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
        D(DBF_REXX, "received reply of a SendRexxCommand() call");
        SHOWVALUE(DBF_REXX, ARexxResultHook);

        if(ARexxResultHook != NULL)
          ARexxResultHook(host, rexxmsg);
        else if(rexxmsg->rm_Result1 != 0)
          ER_NewError(tr(MSG_ER_AREXX_EXECUTION_ERROR), rexxmsg->rm_Args[0], rexxmsg->rm_Result1);
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
/// SendToYAMInstance
// send an ARexx command to an already running instance of YAM
BOOL SendToYAMInstance(char *rxcmd)
{
  BOOL success = FALSE;
  struct MsgPort *replyPort;

  ENTER();

  if((replyPort = AllocSysObjectTags(ASOT_PORT, TAG_DONE)) != NULL)
  {
    struct RexxMsg *rxmsg;

    if((rxmsg = CreateRexxMsg(replyPort, NULL, NULL)) != NULL)
    {
      rxmsg->rm_Action = RXCOMM|RXFF_STRING|RXFF_NOIO;

      if((rxmsg->rm_Args[0] = (APTR)CreateArgstring(rxcmd, strlen(rxcmd))) != 0)
      {
        struct MsgPort *yamPort;

        // look up the ARexx port of the already running instance
        Forbid();

        if((yamPort = FindPort("YAM")) != NULL)
        {
          // now send the message
          PutMsg(yamPort, (struct Message *)rxmsg);

          success = TRUE;
        }

        Permit();

        // if everything went ok we have to wait for the reply before we may continue
        if(success == TRUE)
          WaitPort(replyPort);

        DeleteArgstring((APTR)rxmsg->rm_Args[0]);
      }

      DeleteRexxMsg(rxmsg);
    }

    FreeSysObject(ASOT_PORT, replyPort);
  }

  RETURN(success);
  return success;
}

///
