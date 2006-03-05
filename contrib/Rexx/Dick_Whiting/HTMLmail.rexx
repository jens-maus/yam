/******************************************************************************/
/*                             HTMLmail.rexx                                  */
/*                                                                            */
/*                    for use with YAM 2.0 by Marcel Beck                     */
/*                                                                            */
/*                      Copyright ©1997 by Dick Whiting                       */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/* This script displays the currently selected YAM mail in your browser.      */
/* It is intended for mail that is entirely in HTML format. The logic is to   */
/* look for <html> and copy that and everything to </html> to a temporary     */
/* file and then tell your browser to view that file.                         */
/*----------------------------------------------------------------------------*/
/* Standard Disclaimer: I wrote it, it works for me, I don't guarantee        */
/* that it will do anything productive for anyone else, etc. etc. ;-)         */
/*                                                                            */
/*                                                                            */
/*HOWEVER, if you do find a use for it: I homeschool my kids and they would   */
/*love a postcard from where EVER you live. Instant Geography Lesson;)        */
/*                                                                            */
/*POSTCARDS:    Dick Whiting                                                  */
/*              28590 S. Beavercreek Rd.                                      */
/*              Mulino, Oregon 97042                                          */
/*              USA                                                           */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*                     Dick Whiting <dwhiting@europa.com>                     */
/*                             10 October 1998                                */
/*                                                                            */
/******************************************************************************/
/*                                                                            */
/******************************************************************************/
/*
$VER: 1.0 Copyright ©1998 by Dick Whiting
$AUTHOR: Dick Whiting
$DESCRIPTION: Display HTML mail from Yam2.x using your browser.
*/

options results

/******************************************************************************/
/*         !!! EDIT THESE !!! EDIT THESE !!! EDIT THESE !!!                   */
/******************************************************************************/
browser='IBROWSE'                      /* IBROWSE, VOYAGER, or AWEB only.     */
runbr='C:RUN >NIL: IBROWSE:IBROWSE'    /* this is your browser-- keep quotes  */
newwindow=0                            /* set this to 1 for a NEW window.     */
hideyam=0         /* set this to 1 to iconify YAM prior to opening the browser*/

tfile="T:yammail.html"                 /* temporary file for viewing          */

/******************************************************************************/
/*  Get the file, search for <html> and view it if found                      */
/******************************************************************************/

TRUE=1
FALSE=0
mbody=FALSE
found=FALSE
done=FALSE

Address YAM 'GETMAILINFO FILENAME'
mfile=result

if mfile='RESULT' then do
   Address YAM 'REQUEST "Select a mail to view" "_Okay"'
end
else do
   Call ReadMail
   if found then do
      url='file://localhost/'tfile
      Call ViewIT
   end
   else do
      Address YAM 'REQUEST "Unable to locate HTML portion in mail" "_Okay"'
   end
end

exit

/******************************************************************************/
/* These should work, but you can change them if not.                         */
/******************************************************************************/
ViewIT:

browser=strip(upper(browser))
select
   when browser='IBROWSE' then do
      brscreen='IBROWSE'               /* CASE SENSITIVE browser screen name  */
      portname='IBROWSE'               /* ARexx portname of your browser      */
      portcmd1='GOTOURL "'url'"'       /* for IB, open URL in current window  */  
      portcmd2='NEWWINDOW "'url'"'     /* open URL in a NEW window            */
      showcmd='SHOW'                   /* MUI uniconify command               */
   end
   when browser='VOYAGER' then do
      brscreen='VOYAGER'               /* CASE SENSITIVE browser screen name  */
      portname='VOYAGER'               /* ARexx portname of your browser      */
      portcmd1='OPENURL "'url'"'       /* for IB, open URL in current window  */  
      portcmd2='OPENURL "'url'" NEWWIN'  /* open URL in a NEW window        */
      showcmd='SHOW'                   /* MUI uniconify command               */
   end
   when browser='AWEB' then do   
      brscreen='AWEB.1'                /* CASE SENSITIVE browser screen name  */
      portname='AWEB.1'                /* ARexx portname of your browser      */
      portcmd1='OPEN "'url'"'          /* for IB, open URL in current window  */  
      portcmd2='NEW "'url'"'           /* is there a one for NEW WINDOW ???   */
      showcmd='SHOW'                   /* no idea if this works:)             */
   end
   otherwise do
     Address YAM 'REQUEST "Invalid browser name in script" "OK"'
     exit
   end
end

tricks=0

if ~show('L','rexxsupport.library') then do
   addlib('rexxsupport.library',0,-30)
end

if ~show('L','rexxtricks.library') then do
   if exists('LIBS:rexxtricks.library') then do
      tricks=addlib('rexxtricks.library',0,-30)
   end
end
else tricks=1

if ~show(p,portname) then do
  Address Command runbr '"'url'"'
  do 5 while ~show(p,portname)
     Address Command 'SYS:REXXC/WAITFORPORT 'portname
  end
  if rc=5 then do 
     Address YAM 'SHOW'
     Address YAM 'REQUEST "Could not find "' portname '" port" "OK"'
     exit
  end
end
else do
  interpret 'Address' portname showcmd
  if newwindow then do
     interpret 'Address' portname "'"portcmd2"'"
  end
  else do
     interpret 'Address' portname "'"portcmd1"'"
  end
  if tricks then do
     foo=PUBSCREENTOFRONT(brscreen)
  end
end

if hideyam then do
  Address Command 'C:run >nil: sys:rexxc/rx "Address YAM HIDE"'
end

return

/**************************************************************************/
/*  Read the selected mail looking for the <HTML> line. Save to tfile.    */
/**************************************************************************/
READMAIL:

if open('IN',mfile,'R') then do
   if open('OUT',tfile,'W') then do
      do until (eof('IN') | done)
         linein=readln('IN')
         if ~mbody then do
            if strip(linein='') then mbody=TRUE
         end
         else do
            if found then do
               endpos=pos('</HTML>',upper(linein))
               if endpos>0 then do
                  linein=substr(linein,1,endpos+6)
                  done=true
               end
               foo=writeln('OUT',linein)
            end
            else do
               startpos=pos('<HTML>',upper(linein))
               if startpos>0 then do
                  linein=substr(linein,startpos)
                  foo=writeln('OUT',linein)
                  found=TRUE
               end
            end
         end
      end
      foo=close('OUT')
   end
   else do
      Address YAM 'REQUEST "Unable to open input file"  "_Okay"'
   end
   foo=close('IN')
end
else do
   Address YAM 'REQUEST "Unable to open output file"  "_Okay"'
end

return

