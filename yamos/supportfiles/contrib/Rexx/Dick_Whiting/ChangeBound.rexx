/******************************************************************************/
/*                                                                            */
/*                              ChangeBound                                   */
/*                 Copyright ©1997 by Dick Whiting                            */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*  This program will make MIMED mail readable in Yam that has the boundary   */
/*  set to ALL dashes. Boundary lines and the End Boundary are changed to     */
/*  a new value that Yam will be happy with.                                  */
/*                                                                            */
/*  NOTE: This one, like my other ones, retains the date/timestamp of the     */
/*        original mail and its filenote.                                     */
/*                                                                            */
/*  NOTE: IF someone manages to have a line of DASHES of EXACTLY the correct  */
/*        length of the original boundary, this script and therefore Yam      */
/*        will STILL be confused.                                             */
/*                                                                            */
/*  SUGGESTION: Write to the Digest/List administrator and suggest using a    */
/*        Boundary value that will NOT be accidentally found in someone's     */
/*        mail. A good choice would be something with a datetime stamp and    */
/*        maybe the digest number.                                            */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*  If you use YamTools you can use this one as a TYPE=ONCE to apply to mail  */
/*  selected one at a time from YAM, or TYPE=MAIL to do all mail selected     */
/*  from a YamTools list.  Remember that you CAN define two buttons using     */
/*  same script but with different TYPES.                                     */
/*                                                                            */
/*  If you don't use YamTools--why not;)                                      */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Standard Disclaimer: I wrote it, it works for me, I don't guarantee        */
/* that it will do anything productive for anyone else, etc. etc. ;-)         */
/*                                                                            */
/*HOWEVER, if you DO find a use for it: I homeschool my kids and they         */
/*would love a postcard from where EVER you live.                             */
/*                                                                            */
/*Instant GEOGRAPHY lesson;)                                                  */
/*                                                                            */
/*                                                                            */
/*POSTCARDS:    Dick Whiting                                                  */
/*              28590 S. Beavercreek Rd.                                      */
/*              Mulino, Oregon 97042                                          */
/*              USA                                                           */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*               Address Bug Reports or Comments to:                          */
/*                Dick Whiting <dwhiting@europa.com>                          */
/*                            19 July 1997                                    */
/*                                                                            */
/******************************************************************************/
/*
$VER: 1.0 Copyright ©1997 by Dick Whiting
$AUTHOR: Dick Whiting
$DESCRIPTION: Change Boundary of all dashes to new value        
*/

options results

argpassed=''
parse arg argpassed 

Call Localize

/******************************************************************************/
/*  Find out the necessary information about the file                         */
/******************************************************************************/
Init:

if ~show('L','rexxsupport.library') then do
   addlib('rexxsupport.library',0,-30)
end

Address YAM 'getmailinfo file'
mfile=result
slpos=lastpos('/',mfile)
copos=lastpos(':',mfile)
filename=strip(substr(mfile,max(slpos,copos)+1))
tmpfile = 'T:'||filename                              /* temporary file       */
fstate=statef(mfile)                                  /* file information     */
fbytes=subword(fstate,2,1)                            /* length of file       */
fdate=subword(fstate,5,1)                             /* internal date        */
fmins=subword(fstate,6,1)                             /* minutes since midnite*/
fticks=subword(fstate,7,1)                            /* ticks past minutes   */
fcomm=subword(fstate,8)                               /* old comment          */

fdate=date('E',fdate,'I')                             /* get date in ddmmyy   */
fdate=translate(fdate,'-','/')                        /* convert / to -       */
hh=fmins%60                                           /* get hours            */
hh=right(hh,2,'0')                                    /* force to 2 digits    */
mm=fmins//60                                          /* get minutes          */
mm=right(mm,2,'0')                                    /* force to 2 digits    */
ss=fticks%50                                          /* seconds              */
ss=right(ss,2,'0')                                    /* force it             */
ftime=hh||':'||mm||':'||ss                            /* timestamp rebuilt    */

TRUE=1
FALSE=0
FixBound=FALSE
header=TRUE
outopen=FALSE

newbound='Modified.by.ChangeBound.rexx.'date()'.'time()
newbound=translate(newbound,'..',' :')

/******************************************************************************/
/*  Do the actual modifications to the mail file.                             */
/******************************************************************************/
ChangeBounds:

if open('OUT',tmpfile,'W') then do
   outopen=TRUE
   if open('IN',mfile,'R') then do
      do until eof('IN')
         linein = readln('IN')
         select
            when header & linein='' then header=FALSE
            when header & upper(word(linein,1))='CONTENT-TYPE:' then do
               boundpos=pos('BOUNDARY=',upper(linein))
               if boundpos>0 then do
                  bound1=substr(linein,1,boundpos+8)
                  boundary=substr(linein,boundpos+9)
                  boundary=strip(boundary,'B','"')
                  if boundary~='' & verify('-',boundary)=0 then do 
                     oldbound='--'||boundary
                     oldend=oldbound||'--'
                     boundary=newbound
                     linein=bound1||'"'||newbound||'"'
                     newbound='--'||newbound
                     newend=newbound||'--'
                     FixBound=TRUE
                  end
                  else do
                     leave
                  end
               end
            end
            when strip(linein,'B')=oldbound then linein=newbound
            when strip(linein,'B')=oldend then linein=newend
            otherwise nop
         end
         foo=writeln('OUT',linein)
      end
      foo=close('IN')
      foo=close('OUT')
      outopen=FALSE
      if FixBound then do
         Address Command 'C:Copy 'tmpfile' 'mfile 
         Address Command 'SetDate  ' mfile fdate ftime 
         Address Command 'Filenote ' mfile '"'fcomm'"' 
      end
   end
   if outopen then do
      foo=close('OUT')
   end
   Address Command 'Delete 'tmpfile 'QUIET'
end

if argpassed='' then do
   if FixBound then do
      Address YAM 'request "'donemsg'" "'yesnomsg'"'
      ans=result
      if ans=1 then do
         Address YAM 'mailupdate'
      end
   end
   else do
      Address YAM 'request "'goodBmsg'" "'okmsg'"'
   end
end

exit

/******************************************************************************/
/*  Variables available for localization.                                     */
/******************************************************************************/
Localize:

donemsg='Done..Update Index?'
yesnomsg='Yes|No'
goodBmsg='Good boundary..exiting'
okmsg='OK'

