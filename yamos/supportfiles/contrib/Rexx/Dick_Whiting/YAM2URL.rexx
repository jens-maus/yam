/******************************************************************************/
/*                                                                            */
/*                                  YAM2URL                                   */
/*                                                                            */
/*                    for use with YAM 2.0 by Marcel Beck                     */
/*                                                                            */
/*                    Copyright ©1997-1999 by Dick Whiting                    */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*  This will display a URL using your browser, start the browser if needed,  */
/*  uniconify it if already running. It can also be used to view HTML mime    */
/*  attachments.                                                              */
/*                                                                            */
/*  To use this for double-clicking and going to a URL:                       */
/*     edit the three variables in this script for your configuration         */
/*     YAM Config->ARexx->When double-clicking an URL (last entry in list)    */
/*     set the type to ARexx (cycle gadget)                                   */
/*     set the script string to 'Yam:Rexx/YAM2URL.rexx' (assuming it's here)  */
/*     set open console OFF                                                   */
/*     set wait for termination OFF                                           */
/*     SAVE settings                                                          */
/*     you can now double-click on a URL and go to it.                        */
/*                                                                            */
/*  To use this for displaying included HTML attachments:                     */
/*     YAM Config->Read->MimeViewers                                          */
/*     if there is NOT a type for 'text/html' create a NEW one                */
/*     set viewer to: 'sys:rexxc/rx Yam:Rexx/YAM2URL.rexx %s'                 */
/*     SAVE settings                                                          */
/*     you can now display HTML attachments by selecting 'Display' from the   */
/*        YAM read window and choosing the HTML attachment from the list.     */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/* !!! You need to edit the four values hideyam,browser,runbr, and newwindow. */
/*----------------------------------------------------------------------------*/
/* NOTES:                                                                     */
/*                                                                            */
/* IF YOU HAVE REASSIGNED THE YAM TEMPORARY DIRECTORY USE THIS FORM:          */
/* (This value is specified on the Config->Mixed page.)                       */
/*                                                                            */
/* set viewer to: 'sys:rexxc/rx Yam:Rexx/YAM2URL.rexx file:///%s'             */
/*                                                                            */
/* Also, the '%s' you specify for *MUST* be lowercase. This applies to ALL    */
/* mime prefs, not just the one for Yam2URL. Uppercase '%S' will NOT pass     */
/* the correct information to your mime viewers.                              */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/* If you have the rexxtricks.library installed then the browser screen will  */
/* pop to the front correctly.                                                */
/* You can find this on Aminet/util/rexx/RexxTricks_386.lha                   */
/*                                                                            */
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
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*                     Dick Whiting <dwhiting@europa.com>                     */
/*                             22 March 1999                                  */
/*                                                                            */
/******************************************************************************/
/*
$VER: 1.5 Copyright ©1999 by Dick Whiting
$AUTHOR: Dick Whiting
$DESCRIPTION: Goto URL for Yam2.x
*/

parse arg url
options results

/******************************************************************************/
/*         !!! EDIT THESE !!! EDIT THESE !!! EDIT THESE !!!                   */
/******************************************************************************/
browser='IBROWSE'                      /* IBROWSE, VOYAGER, or AWEB only.     */
runbr='C:RUN >NIL: IBROWSE:IBROWSE'    /* this is your browser-- keep quotes  */
newwindow=0                            /* set this to 1 for a NEW window.     */
hideyam=0         /* set this to 1 to iconify YAM prior to opening the browser*/


/******************************************************************************/
/*  See if it's an html mime attachment.                                      */
/******************************************************************************/
htmltext=0
if upper(left(url,9))='T:YAMMSG-' then do
   htmltext=1
   Address Command 'Copy 'url 'T:yam2url.html QUIET'
   url='file://localhost/T:yam2url.html'
end
else do
   if substr(URL,1,1) ~= '"' then do      /* add quotes if needed Y2p7 change */
      url='"'||url||'"'
   end
end
/******************************************************************************/
/* These should work, but you can change them if not.                         */
/******************************************************************************/
browser=strip(upper(browser))
select
   when browser='IBROWSE' then do
      brscreen='IBROWSE'               /* CASE SENSITIVE browser screen name  */
      portname='IBROWSE'               /* ARexx portname of your browser      */
      portcmd1='GOTOURL ' url          /* for IB, open URL in current window  */  
      portcmd2='NEWWINDOW ' url        /* open URL in a NEW window            */
      showcmd='SHOW'                   /* MUI uniconify command               */
   end
   when browser='VOYAGER' then do
      brscreen='VOYAGER'               /* CASE SENSITIVE browser screen name  */
      portname='VOYAGER'               /* ARexx portname of your browser      */
      portcmd1='OPENURL ' url          /* for IB, open URL in current window  */  
      portcmd2='OPENURL ' url ' NEWWIN'  /* open URL in a NEW window        */
      showcmd='SHOW'                   /* MUI uniconify command               */
   end
   when browser='AWEB' then do   
      brscreen='AWEB.1'                /* CASE SENSITIVE browser screen name  */
      portname='AWEB.1'                /* ARexx portname of your browser      */
      portcmd1='OPEN ' url             /* for IB, open URL in current window  */  
      portcmd2='NEW ' url              /* is there a one for NEW WINDOW ???   */
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
  Address Command runbr url
  do 5 while ~show(p,portname)
     Address Command 'SYS:REXXC/WAITFORPORT 'portname
  end
  if rc=5 then do 
     Address YAM 'SHOW'
     Address YAM 'SCREENTOFRONT'
     Address YAM 'REQUEST "Could not find' portname 'port" "OK"'
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
  Address YAM 'HIDE'
end

exit

