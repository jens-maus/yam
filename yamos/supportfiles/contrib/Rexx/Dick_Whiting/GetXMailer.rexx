/******************************************************************************/
/*                             GetXMailer.rexx                                */
/*                                                                            */
/*               for use with YAM 2.0 by Marcel Beck                          */
/*                                                                            */
/*                 Copyright ©1999 by Dick Whiting                            */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*  Displays the From and X-Mailer line for each selected YAM mail.           */
/*  Set variable maxmail to the number to display at one time. If this number */
/*  is too high nothing will appear to happen or MUI will resize the font     */
/*  in an unpredictable fashion. No damage, but you'll want to restart YAM.   */
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
$VER: 2.1 Copyright ©1999 by Dick Whiting
$AUTHOR: Dick Whiting
$DESCRIPTION: Displays the From and X-Mailer line for each selected YAM mail
$REQUIRES: Works only with Yam2.x by Marcel Beck
*/

maxmail=10                                  /* View groups of this number */
                                            /* Modify to your screen size */
Address YAM
options results
options failat 21

'GETSELECTED STEM' msel.

if msel.num.count=0 | msel.num.count='MSEL.NUM.COUNT' then do
   'REQUEST "Select mail(s) to check for X-Mailer" "OK"'
   exit 
end

'APPBUSY "Scanning mail..."'

xcnt=0
xstr=''

do i=0 to msel.num.count-1
   mpos=msel.num.i
   'MAILINFO' mpos 'STEM' minf.
   mfile=minf.filename
   mfrom=minf.from
   mfrom=translate(mfrom,' ','"') 
   mfrom=translate(mfrom," ","'") 
   Call ReadMail
   xcnt=xcnt+1
   if xcnt=maxmail then do
      'REQUEST "'xstr'" "OK|Cancel"'
      if result=0 then signal DONE
      xstr=''
      xcnt=0
   end
end


if xcnt>0 then do
   Address YAM 'REQUEST "'xstr'" "OK"'
end

DONE:

'APPNOBUSY'

exit

/**************************************************************************/
/*  Read the selected mail looking for the X-Mailer line.                 */
/**************************************************************************/
ReadMail:

TRUE=1
FALSE=0
mbody=FALSE
foundX=FALSE

if open('IN',mfile,'R') then do
   do until eof('IN')
      linein=readln('IN')
      if ~mbody then do
         select
            when upper(substr(linein,1,9))='X-MAILER:' then do
               foundX=TRUE
               leave
            end
            when upper(substr(linein,1,11))='USER-AGENT:' then do
               foundX=TRUE
               leave
            end
            when strip(linein='') then do
               mbody=TRUE
               leave
            end
            otherwise NOP
         end
      end
   end
end

foo=close('IN')

xstr=xstr||'From: 'mfrom||'\n'
if foundX then do   
   xstr=xstr||linein||'\n\n'
end
else do
   xstr=xstr||'X-Mailer: none'||'\n\n'
end

return

