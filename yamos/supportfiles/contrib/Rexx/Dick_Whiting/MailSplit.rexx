/******************************************************************************/
/*                                                                            */
/*                           MailSplit.rexx                                   */
/*                                                                            */
/*               for use with YAM 2.0 by Marcel Beck                          */
/*                                                                            */
/*                 Copyright ©1998-99 by Dick Whiting                         */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*  This one reads the selected mail, gets all the TO: addresses, and creates */
/*  a mail for each.                                                          */
/*                                                                            */
/*  It handles CC: and BCC: addresses, also.                                  */
/*                                                                            */
/*  Create the orginal mail just as you wish it to be sent to each recipient. */
/*  Place the original mail in the outgoing folder.                           */
/*  This script creates an exact copy of the original and handles attachments.*/
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* The default behavior is to set the mail selected for splitting to HELD     */
/* status. If you don't want this, set FORCEHOLD=0                            */
/*                                                                            */
/* The default behavior is to place them in the Outgoing folder for sending   */
/* later. If you want them sent immediately, set AUTOSEND=1.                  */
/*                                                                            */
/* The default behavior is to set the 'Delete after Sending' flag. If this is */
/* NOT what you want, then set the variable DELETEONSEND=0.                   */
/*----------------------------------------------------------------------------*/
/* Changes from previous version:                                             */
/*                                                                            */
/*  1) Changed logic to handle attachments                                    */
/*  2) No temporary files are used now                                        */
/*  3) About 10 times faster than previous version :)                         */
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
/*                            27 July 1999                                    */
/*                                                                            */
/*   Other Scripts & Programs available at http://www.europa.com/~dwhiting/   */
/*                                                                            */
/******************************************************************************/
/*
$VER: 3.1 Copyright ©1998-99 by Dick Whiting
$AUTHOR: Dick Whiting
$DESCRIPTION: Create individual mail for each TO: CC: BCC: recipient
$REQUIRES: Works only with Yam2.x by Marcel Beck
*/

AUTOSEND=0                                  /* SEND them manually         */
DELETEONSEND=1                              /* delete after sending mail  */
FORCEHOLD=1                                 /* force original to HELD     */

options results

ADDRESS YAM

'appbusy' '"Performing mail split...Please wait"'

Call Setup
if rtc then Call Readmail
if rtc then Call Writemails
if rtc & AUTOSEND then Call SendMails

'appnobusy'

exit

/******************************************************************************/
/*  Find out the necessary information about the file, do setup stuff         */
/******************************************************************************/
SETUP:

TRUE=1
FALSE=0
tab='09'x
lf='0A'x
rtc=TRUE

'getfolderinfo num'
fnum=result

'getmailinfo filename'

if fnum~=1 | rc>0 then do
   'request "Select a mail from the OutGoing folder for splitting" "OK"'
   rtc=FALSE
end

mfile=result

if FORCEHOLD then do
   'mailstatus H'
end

if DELETEONSEND then do
   headbuf1='X-YAM-Options: delsent'||lf
end
else headbuf1=''

headbuf2=''

bodybuf.=''
bcnt=1

mto.0=0
i=1

foundto=FALSE
foundfrom=FALSE
foundbody=FALSE

return rtc

/******************************************************************************/
/*  Read the selected mail building list of TO: , CC: addresses.              */
/******************************************************************************/
READMAIL:

if open('IN',mfile,'R') then do
   do until eof('IN') 
      if ~foundbody then do
         linein=readln('IN')
         select
            when upper(substr(linein,1,3))='TO:' then do
               linein=substr(linein,4)
               Call Parsetos
            end
            when upper(substr(linein,1,3))='CC:' then do
               linein=substr(linein,4)
               Call Parsetos
            end
            when upper(substr(linein,1,4))='BCC:' then do
               linein=substr(linein,5)
               Call Parsetos
            end
            when foundto & (left(linein,1)=' ' | left(linein,1)=tab) then do
               Call Parsetos
            end
            when upper(substr(linein,1,5))='FROM:' then do
               headbuf1=headbuf1||linein||lf
               foundfrom=TRUE
               foundto=FALSE
            end
            when strip(linein='') then do
               foundto=FALSE
               foundbody=TRUE
            end
            otherwise do
               foundto=FALSE
               if foundfrom then do
                  headbuf2=headbuf2||linein||lf
               end
               else do
                  headbuf1=headbuf1||linein||lf
               end
            end
         end
      end
      else do
         bodybuf.bcnt=readch('IN',65000)
         if length(bodybuf.bcnt)>0 then bcnt=bcnt+1
      end
   end
   mto.0=i-1
   bodybuf.0=bcnt-1
   foo=close('IN')
end
else do
   'request "Error opening input file" "OK"'
   rtc=FALSE
end

return rtc

/**************************************************************************/
/*                 Parse each TO, CC, BCC value into array                */
/**************************************************************************/
PARSETOS:

   foundto=TRUE

   linein=translate(linein,' ',tab)
   linein=strip(linein)
   do while linein~=''
      parse var linein mto.i ',' linein
      i=i+1                                  /* bump counter for array */
   end

return

/**************************************************************************/
/*              Write a new mail for each TO, CC, BCC found               */
/**************************************************************************/
WRITEMAILS:

do i=1 to mto.0
   'newmailfile'
   fileout.i=result
   if open('OUT',fileout.i,'W') then do
      foo=writech('OUT',headbuf1)
      foo=writech('OUT', "To: "||mto.i||lf)
      foo=writech('OUT',headbuf2||lf)
      do j = 1 to bodybuf.0
         foo=writech('OUT',bodybuf.j)
      end
      foo=close('OUT')
      address command 'filenote "'fileout.i'" "W"'
   end
   else do
      'request "Error in creating output file" "OK"'
      rtc=FALSE
      leave i
   end
end

'mailupdate'
   
return rtc

/******************************************************************************/
/*  If autosend then send each mail created                                   */
/******************************************************************************/
SENDMAILS:

'listselect N'

do i=1 to mto.0
   'setmailfile' fileout.i
   'mailinfo stem' inf.
   sendnums.i=inf.index
end

'setmail' sendnums.1

do i=2 to mto.0
   'listselect' sendnums.i
end

'mailsend' 

return

