/******************************************************************************/
/*                                                                            */
/*                                  YAM2IB                                    */
/*                                                                            */
/*                      Copyright ©1997 by Dick Whiting                       */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/* This is a quickie to display text/html using IBROWSE. You must edit these  */
/* variables prior to using:                                                  */
/*                                                                            */
/******************************************************************************/

runib='run >nil: TEMP:IBROWSE/IBrowse' /* set this to where IB program is     */
ibscreen='IBROWSE'                     /* CASE SENSITIVVE screen name of IB   */
waitsecs=15                            /* wait secs before deleting temp file */       

/******************************************************************************/
/*                                                                            */
/* If you have the rexxtricks.library installed then the IBROWSE screen will  */
/* pop to the front correctly.                                                */
/* You can find this on Aminet/util/rexx/RexxTricks_386.lha                   */
/*                                                                            */
/* You also need to set up a mime type in the configuration like this:        */
/*                                                                            */
/* text/html                                                                  */
/* sys:rexxc/rx yam:rexx/yam2ib.rexx %s                                       */
/*                                                                            */
/* NOTE: Remember to select 'NEW' and THEN enter the information. Also make   */
/* sure that you click 'SAVE';)                                               */
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
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*                     Dick Whiting <dwhiting@europa.com>                     */
/*                              13 January 1997                               */
/*                                                                            */
/******************************************************************************/

parse arg args
options results

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

html=args||'.html'

Address Command 'Copy ' args html

if ~show(p,"IBROWSE") then do
  Address Command runib 'file:///'html
  do 5 while ~show(p,"IBROWSE")
     Address Command 'SYS:REXXC/WAITFORPORT IBROWSE'
  end
  if rc=5 then do 
     say 'Could not find IBROWSE port'
  end
  else do
     foo=delay(waitsecs*50)
  end
end
else do
  Address IBROWSE GOTOURL 'file:///'html
  if tricks then do
     foo=PUBSCREENTOFRONT(ibscreen)
  end
end

Address Command 'Delete ' html '>nil: '

exit

