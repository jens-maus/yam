/******************************************************************************/
/*                                                                            */
/*                             ReDate.rexx                                    */
/*                 Copyright ©1999 by Dick Whiting                            */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*  This one changes the  Date: header in a mail to the date that it          */
/*  was received. Most useful for handling mail sent by people without        */
/*  a valid clock. Not sure what happens if YOU don't have one.               */
/*                                                                            */
/*  This version will process all mail currently selected.                    */
/*                                                                            */
/*  I decided to NOT use the date/timestamp of the file itself, since so      */
/*  many of the scripts that I have seen update it. Instead, I use the        */
/*  filename to determine the date and use the timestamp from the original    */
/*  Date: header to build the new one. This seems to be the best compromise.  */
/*                                                                            */
/*  NOTE: Changes won't be visible in Yam until you do an UPDATE INDEX.       */
/*                                                                            */
/*----------------------------------------------------------------------------*/
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
/*                            04 Oct 1999                                     */
/*                                                                            */
/*   Other Scripts & Programs available at http://www.europa.com/~dwhiting/   */
/*                                                                            */
/******************************************************************************/
/*
$VER: 2.0 Copyright ©1999 by Dick Whiting
$AUTHOR: Dick Whiting
$DESCRIPTION: Change Date: header in mail to the date received for selected mail(s)
$REQUIRES: Works only with Yam2.x by Marcel Beck
*/

ADDRESS YAM
options results

if ~show('L','rexxsupport.library') then do
   addlib('rexxsupport.library',0,-30)
end

Call Localize

'GETSELECTED STEM' msel.

if msel.num.count=0 | msel.num.count='MSEL.NUM.COUNT' then do
   'REQUEST "Select mail(s) to Re-date" "OK"'
   exit 
end

'APPBUSY "Doing re-dating..."'

do i=0 to msel.num.count-1
   'MAILINFO' i 'STEM' minf.
   mfile=minf.filename
   Call ProcessMail
end

Call Cleanup

exit

/******************************************************************************/
/*  Find out the necessary information about the file                         */
/******************************************************************************/
ProcessMail:

slpos=lastpos('/',mfile)
copos=lastpos(':',mfile)
mfilename=strip(substr(mfile,max(slpos,copos)+1))
parse var mfilename fdate '.' rest                    /* fn=internal date     */
dow=substr(date('W',fdate,'I'),1,3)                   /* day of week received */
normdate=date('N',fdate,'I')                          /* date received        */
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

/******************************************************************************/
/*  Read the selected mail and replace the Date: header.                      */
/******************************************************************************/

if open('IN',mfile,'R') & open('OUT','T:fdate.tmp','W') then do
   do until eof('IN')
      linein=readln('IN')
      if substr(linein,1,6)='Date: ' then do
         copos=lastpos(':',linein)                 /* find last : in time     */
         mtime=substr(linein,copos-5)              /* timestamp of sender     */      
         datestamp='Date: '||dow||', '||normdate||' '||mtime
         foo=writeln('OUT',datestamp)
         do until eof('IN')
            blockin=readch('IN',20480)             /* grab BIG chunks now     */
            foo=writech('OUT',blockin)
         end
      end
      else do
         foo=writeln('OUT',linein)
      end
   end
   foo=close('IN')
   foo=close('OUT')
   Address Command 'SetDate T:fdate.tmp 'fdate ftime
   Address Command 'Filenote T:fdate.tmp "'fcomm'"'
   Address Command 'Copy T:fdate.tmp TO ' mfile 'CLONE QUIET'
   Address Command 'Delete T:fdate.tmp QUIET'
end
else do
   'request "Error processing files" "OK"'
   Call Cleanup
   exit
end

return

/**************************************************************************/
/*                         Prompt for Index update                        */
/**************************************************************************/

CLEANUP:

   'APPNOBUSY'

   Address YAM 'request "'donemsg'" "'yesnomsg'"'
   ans=result
   if ans=1 then do
      Address YAM 'mailupdate'
   end

   
return

/******************************************************************************/
/*  Variables available for localization.                                     */
/******************************************************************************/
Localize:

donemsg='Done..Update Index?'
yesnomsg='_OK|_Cancel'

return
