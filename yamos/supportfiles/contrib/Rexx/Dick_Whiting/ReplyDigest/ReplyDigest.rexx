/******************************************************************************/
/*                        ReplyDigest.rexx                                    */
/*                                                                            */
/*               for use with YAM 2.0 by Marcel Beck                          */
/*                                                                            */
/*                 Copyright ©1999 by Dick Whiting                            */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*  Allows you to reply to a selected message within a digest.                */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Standard Disclaimer: I wrote it, it works for me, I don't guarantee        */
/* that it will do anything productive for anyone else, etc. etc. ;-)         */
/*                                                                            */
/* HOWEVER, if you DO find a use for it: I homeschool my kids and they        */
/* would love a postcard from where EVER you live.                            */
/*                                                                            */
/* Instant GEOGRAPHY lesson;)                                                 */
/*                                                                            */
/*                                                                            */
/* POSTCARDS:    Dick Whiting                                                 */
/*               28590 S. Beavercreek Rd.                                     */
/*               Mulino, Oregon 97042                                         */
/*               USA                                                          */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*               Address Bug Reports or Comments to:                          */
/*                Dick Whiting <dwhiting@europa.com>                          */
/*                          05 Nov 1999                                       */
/*                                                                            */
/*   Other Scripts & Programs available at http://www.europa.com/~dwhiting/   */
/*                                                                            */
/******************************************************************************/
/*
$VER: 1.0 Copyright ©1999 by Dick Whiting
$AUTHOR: Dick Whiting
$DESCRIPTION: Reply to a selected message within a digest                          
$REQUIRES: Works only with Yam2.x by Marcel Beck
*/

/**************************************************************************/
/*                  Modify these if desired or necessary                  */
/**************************************************************************/

outfolder=1                                 /* Number of OutGoing folder  */
deletetemp=1                                /* 1=Delete tempfile when done*/

/**************************************************************************/
/*             Maximum number of lines to display in requester            */
/*     If this number is TOO large you will see nothing on the screen     */
/**************************************************************************/

maxlines=20                                 /* Display lines for screen   */
                                            /* Modify to your screen size */

/**************************************************************************/
/*                     Parsing information for digests                    */
/**************************************************************************/

digest.0=4                                  /* Number of digest defs      */

/**************************************************************************/
/*                   Should work for all OneList digests                  */
/**************************************************************************/

digest.1.replyto="@onelist.com"             /* look at Reply-to: substr   */
digest.1.from=""                            /* look for From substring    */
digest.1.keyfld="Subject"                   /* look in SUBJECT: for key   */
digest.1.keyval="Digest Number"             /* key string to look for     */
digest.1.sindx="Topics in today's digest:"  /* Line preceeding index      */
digest.1.pindx=1                            /* index processing type      */
digest.1.eindx=""                           /* Index ends at null line    */
digest.1.lindx=2                            /* 2 lines per index entry    */
digest.1.ptype=1                            /* msg processing type        */
digest.1.divs="Message: #"                  /* Unique str for msg start   */
                                            /* '#' is the message number  */
digest.1.deldiv=1                           /* delete nn lines starting   */
                                            /* with line divider is in    */
                                            /* to get to actual message   */
digest.1.ecomp=1                            /* compare for endmsg lines   */
digest.1.ends=copies("_",79)                /* Line ending a message      */

/**************************************************************************/
/*                   Should work for all Egroups digests                  */
/**************************************************************************/

digest.2.replyto="@egroups.com"
digest.2.from=""
digest.2.keyfld="Subject"                 
digest.2.keyval=" digest"
digest.2.sindx=copies('-',65)
digest.2.pindx=2           
digest.2.eindx=copies('-',65)
digest.2.lindx=1       
digest.2.ptype=2          
digest.2.divs="------------------------------ message "
digest.2.deldiv=2
digest.2.ecomp=2
digest.2.ends="------------------------------"

/**************************************************************************/
/* These will support both Yam and I-Amiga (probably other LISTSERV also) */
/*         For these to work set your list options as DIGESTS MIME        */
/**************************************************************************/

/**************************************************************************/
/* This one works with a partial day's digest and MUST precede the full   */
/**************************************************************************/

digest.3.replyto=""
digest.3.from="Automatic digest processor"
digest.3.keyfld="Subject"                 
digest.3.keyval=" - Unfinished"
digest.3.sindx="Topics collected thus far:"
digest.3.pindx=3           
digest.3.eindx=""       
digest.3.lindx=2        
digest.3.ptype=2       
digest.3.divs="MIME"
digest.3.deldiv=1
digest.3.ecomp=2
digest.3.ends="MIME"

/**************************************************************************/
/*                 This one works with a full day's digest                */
/**************************************************************************/

digest.4.replyto=""
digest.4.from="Automatic digest processor"
digest.4.keyfld="Subject"                 
digest.4.keyval=" Digest -"   
digest.4.sindx="Topics of the day:"
digest.4.pindx=3           
digest.4.eindx=""       
digest.4.lindx=2        
digest.4.ptype=2          
digest.4.divs="MIME"
digest.4.deldiv=1
digest.4.ecomp=2
digest.4.ends="MIME"

/**************************************************************************/
/*                                                                        */
/*                Index Processing Types (digest.n.pindx)                 */
/*                                                                        */
/* 1 = Index lists ALL messages AND numbers them from 1                   */
/* 2 = Index lists ALL messages but does NOT number them from 1 OR        */
/*     does NOT number them.                                              */
/* 3 = Index lists MULTIPLE messages as a single entry                    */
/*                                                                        */
/*           Start of Message Processing Types (digest.n.ptype)           */
/*                                                                        */
/* 1 = Message start has an index number STARTING from 1 in it            */
/* 2 = Must COUNT occurences of string to find message start              */
/*                                                                        */
/*            End of Message Processing Types (digest.n.ecomp)            */
/*                                                                        */
/* 1 = Ending line EQUALS this value                                      */
/* 2 = Ending line CONTAINS this value (make it UNIQUE)                   */
/*                                                                        */
/**************************************************************************/

/**************************************************************************/
/*                      Nothing to tailor beyond here                     */
/**************************************************************************/

Address YAM
options results
options failat 21

TRUE=1
FALSE=0

Call TestMail 
Call Readmail
Call CheckDivs
Call BuildIndex
Call DisplayIndex
Call GetMessage
Call ReplyMail

DONE:
   
exit

/**************************************************************************/
/*         Test email for correct From/Replyto and keyfield value         */
/**************************************************************************/

TestMail:

   'FOLDERINFO STEM' finf.
   curfolder=finf.number
   
   'MAILINFO STEM' minf.
   
   mfile=minf.filename
   mfrom=minf.from
   msubj=minf.subject
   mrepl=minf.replyto
   msize=minf.size
   
   mailmatch=FALSE
   
   do i=1 to digest.0                       /* See if a known address     */
      select
         when digest.i.replyto~='' & pos(digest.i.replyto,mrepl)>0 then do
            Call TestKey
            if mailmatch then leave i
         end
         when digest.i.from~='' & pos(digest.i.from,mfrom)>0 then do
            Call TestKey
            if mailmatch then leave i
         end
         otherwise nop
      end
   end
   
   if ~mailmatch then do
      'REQUEST "Unknown Digest Format\nYou may need to update script" "OK"'
      signal DONE 
   end
   
   digtype=i 

return

/**************************************************************************/
/*                      Test the key field for value                      */
/**************************************************************************/

TestKey:

   select                                   /* Check for key phrase       */
      when digest.i.keyfld='Subject' & pos(digest.i.keyval,msubj)>0 then do
         mailmatch=TRUE
      end
      otherwise nop
   end

return

/**************************************************************************/
/*  Read the selected mail into an array of 64K blocks.                   */
/**************************************************************************/

ReadMail:

   mstr.=''
   mblks=1
   blksize=4000                             /* Compromise for later speed */
   blkcnt=1 + msize%blksize
   
   if open('IN',mfile,'R') then do
      do blkcnt
         blkin=readch('IN',blksize)
         lastlf=lastpos('0A'x,blkin)
         mstr.mblks=mstr.mblks||substr(blkin,1,lastlf)
         mblks=mblks+1
         mstr.mblks=substr(blkin,lastlf+1)
      end
      foo=close('IN')
   end
   else do
      'REQUEST "Unable to open mail file" "OK"'
      signal DONE
   end
   
return

/**************************************************************************/
/*             Check if need to handle MIME type dividers                 */
/**************************************************************************/

CheckDivs:

   if upper(digest.digtype.divs)~="MIME" then return

   bpos=pos("boundary=",mstr.1)             /* assume is in block 1       */
   if bpos=0 then do
      'REQUEST "Unable to locate MIME boundary= statement" "OK"'
      signal DONE
   end
   mstr.1=substr(mstr.1,bpos+10)            /* skip boundary=" part       */
   parse var mstr.1 bound '"' mstr.1        /* get boundary string        */

   digest.digtype.divs='--'||bound          /* build boundary divider     */
   digest.digtype.deldiv=1                  /* delete boundary line only  */
   digest.digtype.ecomp=2                   /* force substr end compare   */
   digest.digtype.ends='--'||bound          /* use same for end compare   */

return

/**************************************************************************/
/*                         Construct the Index array                      */
/**************************************************************************/

BuildIndex:

   'APPBUSY "Building index...please wait"'

   idx.0=0
   idxcnt=0

   do i=1 to blkcnt                         /* locate start of index blk  */
      idxstart=pos(digest.digtype.sindx,mstr.i)
      if idxstart=0 then iterate
      mstr.i=substr(mstr.i,idxstart)
      parse var mstr.i mline '0A'x mstr.i   /* mline with index key       */
      parse var mstr.i mline '0A'x mstr.i   /* mline AFTER index key      */
      if length(mstr.i)=0 then i=i+1        /* mline last line in block   */

      do j=i to blkcnt
         do while length(mstr.j)>0             /* find first non-null line*/
            if strip(mline)~='' then leave j   /* found non-null line     */
            parse var mstr.j mline '0A'x mstr.j   /* next index line      */
         end
      end

      if strip(mline)='' then do            /* no index entries found     */
         'APPNOBUSY'
         'REQUEST "Unable to parse index" "OK"'
         signal DONE
      end

      if digest.digtype.pindx=3 then do     /* Need to build by hand      */
         idxblk=j                           /* start-of-index block       */
         Call BldByHand
         leave i
      end

      do k=j to blkcnt                      /* build idx array looking... */
         idxblk=k                           /* end-of-index block         */
         do while length(mstr.k)>0          /* ...for end-of-index line   */
            idxcnt=idxcnt+1 
            mline=translate(mline," ","'")
            mline=translate(mline,' ','"')
            select
               when digest.digtype.pindx=1 then idx.idxcnt=mline
               when digest.digtype.pindx=2 then do
                  if digest.digtype.lindx>1 then do
                     xcnt=digest.digtype.lindx
                     if (idxcnt//xcnt)=1 then do 
                        idx.idxcnt=right(1+(idxcnt%xcnt),2,' ')||'   '||mline
                     end
                     else idx.idxcnt='     '||mline
                  end
                  else idx.idxcnt=right(idxcnt,2,' ')||'   '||mline                  
               end
               otherwise do
                  'REQUEST "Invalid value for digest."'digtype'".pindx" "OK"'
                  'APPNOBUSY'
                  signal DONE
               end
            end
            parse var mstr.k mline '0A'x mstr.k
            if mline=digest.digtype.eindx then leave i    /* end-of-index */
         end
      end
      leave i
   end

   if idxstart=0 then do
      'APPNOBUSY'
      'REQUEST "Start of index not found" "OK"'
      signal DONE
   end

   'APPNOBUSY'

return

/**************************************************************************/
/*   Build index by looking for message starts and From: Subject: lines   */
/**************************************************************************/

BldByHand:

   mpos=1
   
   do k=j to blkcnt                         /* build idx array looking... */
      mpos=pos(digest.digtype.divs,mstr.k,1)
      do while mpos>0
         do i=k to blkcnt
            fpos=pos("From: ",mstr.i,mpos)
            if fpos=0 then mpos=1
            else do 
               fline=substr(mstr.i,fpos+6,pos('0A'x,mstr.i,fpos)-(fpos+6))
               fline=translate(fline,' ','"')
               fline=translate(fline,' ',"'")
               fline=strip(fline)
               leave i
            end
         end
         do j=k to blkcnt
            spos=pos("Subject: ",mstr.j,mpos)
            if spos=0 then mpos=1
            else do 
               sline=substr(mstr.j,spos+9,pos('0A'x,mstr.j,spos)-(spos+9))
               sline=translate(sline,' ','"')
               sline=translate(sline,' ',"'")
               sline=strip(sline)
               leave j
            end
         end
         if fpos>0 & spos>0 then do
            idxcnt=idxcnt+1
            idx.idxcnt=right(1+idxcnt%2,2,' ')||'. '||sline
            idxcnt=idxcnt+1
            idx.idxcnt='      From: '||fline
            select
               when i=j then mpos=max(fpos,spos)
               when i>j then mpos=fpos
               when j>i then mpos=spos
               otherwise nop
            end
            k=max(i,j)
            mpos=pos(digest.digtype.divs,mstr.k,mpos)
         end
         else do
            leave k
         end
      end
   end

return

/**************************************************************************/
/*                 Display the index requester(s) for user                */
/**************************************************************************/

DisplayIndex:
         
   reqstr=''
   prompt=''
   getmsgnum=-1

   do i=1 to idxcnt by maxlines
      do j=i to maxlines+i-1

         if j>idxcnt then leave i
         if reqstr~='' then reqstr=reqstr||'\n'||idx.j
                       else reqstr=idx.j

         if digest.digtype.lindx>1 then do
            if (j//digest.digtype.lindx)=1 then do 
               prompt=prompt||(j%digest.digtype.lindx)+1||'|'
            end
            else nop
         end
         else prompt=prompt||j||'|'

      end

      if j<idxcnt then prompt='Next|'||prompt         /* need NEXT button */
      prompt=prompt||'Cancel'                         /* add CANCEL option*/
      'REQUEST "'reqstr'" "'prompt'"'
      response=result

      select
         when response=0 then signal DONE
         when j<idxcnt & response=1 then getmsgnum=0  /* NEXT selected    */
         otherwise do
            if j<idxcnt then response=response-1      /* skip NEXT button */
            if digest.digtype.lindx>1 then do
               getmsgnum=(i%digest.digtype.lindx)+response
            end
            else do
               getmsgnum=i+response-1
            end
            signal DONEIDX
         end
      end
      reqstr=''
      prompt=''
   end
   
   if reqstr~='' then do
      prompt=prompt||'Cancel'

      'REQUEST "'reqstr'" "'prompt'"'
      response=result

      if response=0 then signal DONE
      if response>0 then do
         if digest.digtype.lindx>1 then do
            getmsgnum=(i%digest.digtype.lindx)+response
         end
         else do
            getmsgnum=i+response-1
         end
      end
   end

DONEIDX:

if getmsgnum<0 then do
   'REQUEST "You may have variable MAXLINES set too high\nSee comments in script" "OK"'
   signal DONE
end

return

/**************************************************************************/
/*                    Locate the selected message start                   */
/**************************************************************************/

GetMessage:

   'APPBUSY "Scanning for message...please wait"'

   msgstart=digest.digtype.divs

   linecnt=0
   msgout.=''

   select
      when digest.digtype.ptype=1 then Call GetByString
      when digest.digtype.ptype=2 then Call GetByCount
      otherwise do
         'APPNOBUSY'
         'REQUEST "Invalid value for digest.'digtype'.ptype" "OK"'
         signal DONE
      end
   end

   if startpos=0 then do
      'APPNOBUSY'
      'REQUEST "Unable to locate START of message" "OK"'
      signal DONE
   end

   delcnt=0

   do while delcnt < digest.digtype.deldiv  /* Handle lines to delete     */
      do i=msgblk to blkcnt
         do while length(mstr.i)>0
            parse var mstr.i mline '0A'x mstr.i
            delcnt=delcnt+1
            if delcnt>=digest.digtype.deldiv then leave i
         end
      end
   end

   foundheader=FALSE
   foundbody=FALSE

   do j=msgblk to blkcnt                    /* Do lines of message til END*/
      do while length(mstr.j)>0 
         parse var mstr.j mline '0A'x mstr.j
         select                             /* Check for msg end lines    */
            when digest.digtype.ecomp=1 then do
               if mline=digest.digtype.ends then leave j
            end
            when digest.digtype.ecomp=2 then do
               if pos(digest.digtype.ends,mline)>0 then leave j
            end
            otherwise nop
         end
         select
            when ~foundheader then do
               if strip(mline)='' then iterate
               else do
                  mline=strip(mline)
                  foundheader=TRUE 
               end
            end
            when foundheader & ~foundbody then do
               mline=strip(mline)
               if mline='' then foundbody=TRUE
            end                     
            otherwise nop
         end
         if ~foundbody then do              /* handle broken header lines */
            firstword=word(mline,1)
            if substr(firstword,length(firstword),1)~=':' then do
               msgout.linecnt=msgout.linecnt||mline
               iterate
            end
         end
         linecnt=linecnt+1                
         msgout.linecnt=mline    
      end
   end

   'APPNOBUSY'

return

/**************************************************************************/
/*      Locate message to reply to by a string with an imbedded number    */
/**************************************************************************/

GetByString:

   numpos=pos('#',msgstart)
   if numpos>0 then do
      msgstart=delstr(msgstart,numpos,1)
      msgstart=insert(getmsgnum,msgstart,numpos-1)
      msgstart=msgstart
   end
   else do
      'APPNOBUSY'
      'REQUEST "Message divider must include a message number" "OK"'
      signal DONE
   end

   do msgblk=idxblk to blkcnt               /* Locate start of message #  */
      startpos=pos(msgstart,mstr.msgblk)
      if startpos>0 then do
         mstr.msgblk=substr(mstr.msgblk,startpos)
         leave msgblk
      end
   end
  
return

/**************************************************************************/
/*      Locate message to reply to by counting until nth string           */
/**************************************************************************/

GetByCount:

   foundcnt=0

   do msgblk=idxblk to blkcnt 
      do j=1 to getmsgnum
         startpos=pos(msgstart,mstr.msgblk)
         if startpos>0 then do
            foundcnt=foundcnt+1
            if foundcnt=getmsgnum then do
               mstr.msgblk=substr(mstr.msgblk,startpos)
               leave msgblk
            end
            else do                         /* count only FIRST in line   */
               mstr.msgblk=substr(mstr.msgblk,startpos)
               parse var mstr.msgblk foo '0A'x mstr.msgblk
            end
         end
      end
   end

return

/**************************************************************************/
/*       Write GetMessage temporarilly to Outgoing and reply to it        */
/**************************************************************************/

ReplyMail:

   if linecnt<4 then do                     /* Didn't get a complete mail */
      'REQUEST "Parsing of message incomplete" "OK"'
      signal DONE
   end

   'SETFOLDER' Outfolder

   'NEWMAILFILE' Outfolder
   tempmail=result
   
   if open('OUT',tempmail,'W') then do
      if mrepl~='' then foo=writeln('OUT','Reply-to: '||mrepl)
      foo=writeln('OUT','To: **Tempfile**')
      do i=1 to linecnt
         foo=writeln('OUT',msgout.i)
      end 
      foo=close('OUT')
      'MAILUPDATE'
      'SETMAILFILE' tempmail
      'MAILREPLY'
      'SETMAILFILE' tempmail
      if deletetemp then 'MAILDELETE ATONCE FORCE'
   end
   else do
      'REQUEST "Unable to write temporary file" "OK"'
   end   

   'SETFOLDER' curfolder
   'SETMAILFILE' mfile
   
return

