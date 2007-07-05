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

#include "YAM_mime.h"
#include "YAM_rexx.h"
#include "YAM_rexx_rxcl.h"
#include "YAM_utilities.h"

#include "Debug.h"

#define RexxPortBaseName "YAM"
#define RexxMsgExtension "YAM"

#if INCLUDE_VERSION >= 44
#define REXXMSG(msg) msg
#else
#define REXXMSG(msg) &msg->rm_Node
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

static void (*ARexxResultHook)( struct RexxHost *, struct RexxMsg * ) = NULL;

/// ReplyRexxCommand
void ReplyRexxCommand(struct RexxMsg *rexxmessage, long primary, long secondary, char *result)
{
   if(isFlagSet(rexxmessage->rm_Action, RXFF_RESULT))
   {
      if( primary == 0 )
      {
         secondary = result
            ? (long) CreateArgstring( result, (ULONG)strlen(result) )
            : (long) NULL;
      }
      else
      {
         if( primary > 0 )
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
   }
   else if( primary < 0 )
      primary = -primary;
   
   rexxmessage->rm_Result1 = primary;
   rexxmessage->rm_Result2 = secondary;
   ReplyMsg( (struct Message *) rexxmessage );
}

///
/// FreeRexxCommand
void FreeRexxCommand( struct RexxMsg *rexxmessage )
{
   if(!rexxmessage->rm_Result1 && rexxmessage->rm_Result2)
      DeleteArgstring((APTR)rexxmessage->rm_Result2);

   if( rexxmessage->rm_Stdin &&
      rexxmessage->rm_Stdin != Input() )
      Close( rexxmessage->rm_Stdin );

   if( rexxmessage->rm_Stdout &&
      rexxmessage->rm_Stdout != rexxmessage->rm_Stdin &&
      rexxmessage->rm_Stdout != Output() )
      Close( rexxmessage->rm_Stdout );

   DeleteArgstring((APTR)ARG0(rexxmessage));
   DeleteRexxMsg( rexxmessage );
}

///
/// CreateRexxCommand
static struct RexxMsg *CreateRexxCommand(struct RexxHost *host, char *buff, BPTR fh, int addFlags)
{
   struct RexxMsg *rexx_command_message;

   if( (rexx_command_message = CreateRexxMsg( host->port,
      RexxMsgExtension, host->port->mp_Node.ln_Name)) == NULL )
   {
      return( NULL );
   }

   if( (rexx_command_message->rm_Args[0] =
      (APTR)CreateArgstring(buff, strlen(buff))) == NULL )
   {
      DeleteRexxMsg(rexx_command_message);
      return( NULL );
   }

   rexx_command_message->rm_Action = RXCOMM | RXFF_RESULT | addFlags;
   rexx_command_message->rm_Stdin  = fh;
   rexx_command_message->rm_Stdout = fh;

   return( rexx_command_message );
}

///
/// CommandToRexx
static struct RexxMsg *CommandToRexx( struct RexxHost *host, struct RexxMsg *rexx_command_message )
{
   struct MsgPort *rexxport;

   Forbid();

   if( (rexxport = FindPort(RXSDIR)) == NULL )
   {
      Permit();
      return( NULL );
   }

   PutMsg( rexxport, &rexx_command_message->rm_Node );
   
   Permit();
   
   ++host->replies;

   return( rexx_command_message );
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
  {
    // not all SDKs do supply that new flag already
    #ifndef RXFF_SCRIPT
    #define RXFF_SCRIPT (1 << 21)
    #endif

    rcm = CreateRexxCommand(host, buff, fh, RXFF_SCRIPT);
  }
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
         
      while((rexxmsg = (struct RexxMsg *) GetMsg(host->port)) != NULL)
      {
        if(rexxmsg->rm_Node.mn_Node.ln_Type == NT_REPLYMSG)
        {
          if(!rexxmsg->rm_Args[15] )
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
    while((rexxmsg = (struct RexxMsg *) GetMsg(host->port)) != NULL)
      ReplyRexxCommand(rexxmsg, -20, (long)"Host closing down", NULL);
      
    if(isFlagClear(host->flags, ARB_HF_USRMSGPORT))
      DeleteMsgPort( host->port );
  }
   
  if(host->rdargs != NULL)
    FreeDosObject(DOS_RDARGS, host->rdargs);
  FreeVec(host);

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
   
  if((host = AllocVec(sizeof(struct RexxHost), MEMF_SHARED|MEMF_CLEAR)) != NULL)
  {
    strlcpy(host->portname, basename, sizeof(host->portname));
   
    if((host->port = usrport) != NULL)
    {
      SET_FLAG(host->flags, ARB_HF_USRMSGPORT);
    }
    else if((host->port = CreateMsgPort()) != NULL)
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
        DeleteMsgPort(host->port);
      FreeVec(host);
      host = NULL;
    }
  }
   
  RETURN(host);
  return(host);
}

///

// state machine for FindRXCommand()
/// scmp
static char *scmp(char *inp, const char *str)
{
  while(*str != '\0' && *inp != '\0')
    if(*inp++ != *str++)
      return NULL;
   
  // return the remaining string
  return inp;
}

///
/// find
static int find( char *input )
{
  struct arb_p_state *st = arb_p_state;
  struct arb_p_link *ad;
  char *ni;
  char tmp[36];
  const char *s;
   
  ni = tmp;
  while(*input != '\0' && ni-tmp < 32)
  {
    *ni++ = toupper(*input);
    ++input;
  }
  *ni = 0;
  input = tmp;
   
  while(*input != '\0')
  {
    // did we reach the terminal state?
    if(!st->pa)
    {
      if(*input != '\0')
        return -1;
      else
        return st->cmd;
    }
      
    // where to continue?
    ni = 0;
    for(ad = st->pa; (s = ad->str); ad++)
    {
      // the links are sorted descendant
      if(*input > *s)
        break;
         
      if(*input == *s)
        if((ni = scmp(input+1, s+1)) != NULL)
          break;
    }
      
    // nowhere to continue
    if(ni == NULL)
      return -1;
      
    // state check
    st = arb_p_state + ad->dst;
    input = ni;
  }
   
  return st->cmd;
}

///
/// FindRXCommand
static struct rxs_command *FindRXCommand(char *com)
{
   int index;
   struct rxs_command *cmd = NULL;

   ENTER();
   SHOWSTRING(DBF_REXX, com);
   
   if((index = find(com)) != -1)
     cmd = rxs_commandlist + index;
   
   RETURN(cmd);
   return cmd;
}

///
/// ParseRXCommand
static struct rxs_command *ParseRXCommand(char **arg)
{
  char com[256], *s, *t;
   struct rxs_command *cmd;
   
  ENTER();

  s = *arg;
  t = com;
   
  while(*s != '\0' && *s != ' ' && *s != '\n' && (unsigned int)(t - com + 1) < sizeof(com))
    *t++ = *s++;
   
  *t = '\0';
  while(*s == ' ')
    ++s;
  *arg = s;

  SHOWSTRING(DBF_REXX, com);
  SHOWSTRING(DBF_REXX, *arg);

  cmd = FindRXCommand(com);

  RETURN(cmd);
  return cmd;
}

///
/// CreateVAR
static char *CreateVAR( struct rxs_stemnode *stem )
{
   char *var;
   struct rxs_stemnode *s;
   long size = 0;
   
   if( !stem || stem == (struct rxs_stemnode *) -1L )
      return( (char *) stem );
   
   for( s = stem; s; s = s->succ )
      size += strlen( s->value ) + 1;
   
   if( !(var = AllocVec( size + 1, MEMF_SHARED )) )
      return( (char *) -1 );
   
   *var = '\0';
   
   for( s = stem; s; s = s->succ )
   {
      strlcat(var, s->value, size+1);
      if(s->succ)
         strlcat(var, " ", size+1);
   }
   
   return( var );
}

///
/// new_stemnode
static struct rxs_stemnode *new_stemnode( struct rxs_stemnode **first, struct rxs_stemnode **old )
{
   struct rxs_stemnode *new;
   
   if( !(new = AllocVec(sizeof(struct rxs_stemnode), MEMF_SHARED|MEMF_CLEAR)) )
   {
      return( NULL );
   }
   else
   {
      if( *old )
      {
         (*old)->succ = new;
         (*old) = new;
      }
      else
      {
         *first = *old = new;
      }
   }
   
   return( new );
}

///
/// free_stemlist
static void free_stemlist( struct rxs_stemnode *first )
{
   struct rxs_stemnode *next;
   
   if( (long) first == -1 )
      return;
   
   for( ; first; first = next )
   {
      next = first->succ;

      if(first->name)
        free(first->name);

      if(first->value)
        free(first->value);

      FreeVec( first );
   }
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

   if( !(argb = AllocVec((ULONG)strlen((char *) ARG0(rexxmsg)) + 2, MEMF_SHARED)) )
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
            if( argb ) FreeVec( argb );

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
      rc2 = (long) "Command disabled";
      goto drc_cleanup;
   }
   
   // get memory for the arguments
   (rxc->function)(host, (void **)(APTR)&array, RXIF_INIT, rexxmsg);
   cargstr = AllocVec((ULONG)(rxc->args ? 15+strlen(rxc->args) : 15), MEMF_SHARED );
   
   if( !array || !cargstr )
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
               
               FreeVec( result );
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
               
               if( result && (long) result != -1 )
                  FreeVec( result );
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
   ReplyRexxCommand( rexxmsg, rc, rc2, result );
   
   // free the memory
   if( result ) FreeVec( result );
   FreeArgs( host->rdargs );
   if( cargstr ) FreeVec( cargstr );
   if( array ) (rxc->function)( host, (void **)(APTR)&array, RXIF_FREE, rexxmsg );
   if( argb ) FreeVec( argb );

   LEAVE();
}

///
/// ARexxDispatch
void ARexxDispatch( struct RexxHost *host )
{
   struct RexxMsg *rexxmsg;

   ENTER();

   while((rexxmsg = (struct RexxMsg *) GetMsg(host->port)))
   {
      if( (rexxmsg->rm_Action & RXCODEMASK) != RXCOMM )
      {
         // No Rexx-Message
         ReplyMsg( (struct Message *) rexxmsg );
      }
      else if( rexxmsg->rm_Node.mn_Node.ln_Type == NT_REPLYMSG )
      {
         struct RexxMsg *org = (struct RexxMsg *) rexxmsg->rm_Args[15];
         
         if( org )
         {
            // Reply to a forwarded Msg
            if( rexxmsg->rm_Result1 != 0 )
            {
               // command unknown
               ReplyRexxCommand( org, 20, ERROR_NOT_IMPLEMENTED, NULL );
            }
            else
            {
               ReplyRexxCommand( org, 0, 0, (char *) rexxmsg->rm_Result2 );
            }
         }
         else
         {
            // reply to a SendRexxCommand()-Call
            if( ARexxResultHook )
               ARexxResultHook( host, rexxmsg );
         }
         
         FreeRexxCommand( rexxmsg );
         --host->replies;
      }
      else if( ARG0(rexxmsg) )
      {
         DoRXCommand( host, rexxmsg );
      }
      else
      {
         ReplyMsg( (struct Message *) rexxmsg );
      }
   }

   LEAVE();
}
///

