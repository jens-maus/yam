/******************************************************************************/
/*                                                                            */
/*                           UnMime.rexx                                      */
/*                 Copyright ©1997 by Dick Whiting                            */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*        This version works with either MuiRexx 2.2 or MuiRexx 3.0           */
/*----------------------------------------------------------------------------*/
/* This one attempts to identify the Quoted-Printable portions of a mail      */
/* and converts it back to 8bit text. It also handles header lines with       */
/* the ISO-8859-1?Q type encoding (only single lines are done correctly).     */
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
/*                          15 June 1997                                      */
/*                                                                            */
/******************************************************************************/
/*
$VER: 1.1 Copyright ©1997 by Dick Whiting
$AUTHOR: Dick Whiting
$DESCRIPTION: Change Quoted-Printable back to text and rewrap.
*/

options results
options failat 21

if ~show('L','rexxsupport.library') then do
   addlib('rexxsupport.library',0,-30)
end

Address YAM 'getmailinfo file'              /* find which mail to unmime  */
mfile=result

TRUE=1
FALSE=0
choplen=76                                  /* chop lines to this length  */

ofile=substr(mfile,lastpos('/',mfile)+1)    /* create a tempfile name     */
bakup='T:'||ofile||'.bak'
ofile='T:'||ofile||'.unmime'

Call ReadMail
Call SaveFile

exit

/**************************************************************************/
/*                        Read the mail file into array                   */
/**************************************************************************/

/**************************************************************************/
/*       From: =?iso-8859-1?Q?Ren=E9?= LeBlanc <leblancr@amug.org>        */
/**************************************************************************/

ReadMail:     

   header=TRUE                            /* start with header lines      */
   qlines.=''                             /* array for quoted lines       */
   mimecnt=0                              /* no mime lines yet            */
   mimeline=''                            /* init empty                   */
   mime=FALSE                             /* NOT quoted-printable         */
   quotehead='CONTENT-TRANSFER-ENCODING: QUOTED-PRINTABLE'
   xferhead='CONTENT-TRANSFER-ENCODING:'  /* other xfer types             */
   CTE8bit='Content-Transfer-Encoding: 8bit'
   isomime='=?ISO-8859-1?Q?'              /* mimed sections of a line     */
   isoend='?='

   if open('IN',mfile,'R') & open('OUT',ofile,'W') then do
      do while ~eof('IN')
         linein=readln('IN')
         select
            when upper(linein)=quotehead then do
               linein=CTE8bit             /* replace CTE line with 8bit   */
               mime=TRUE
            end
            when pos(xferhead,upper(linein))=1 then mime=FALSE
            when pos(isomime,upper(linein))>0 then do
               testword=word(linein,1)
               if substr(testword,length(testword),1)=':' then do
                  Call DoQencode
               end
            end
            when header & linein='' then header=FALSE
            otherwise nop
         end
         select
            when ~mime & mimeline='' then do
               foo=writeln('OUT',linein)        /* write lines to output  */
            end
            when ~mime & mimeline~='' then do
               Call ProcessMime
               foo=writeln('OUT',linein)        /* write lines to output  */
            end
            when mime then do
               select
                  when linein='' then do
                     if ~eof('IN') then do 
                        Call ProcessMime
                     end 
                  end
                  when substr(linein,length(linein),1)='=' then do
                     mimeline=mimeline||substr(linein,1,length(linein)-1)
                  end
                  otherwise do
                     mimeline=mimeline||linein
                     Call ProcessMime
                  end
               end
            end
            otherwise do
               say 'Found otherwise in ReadMail..quitting'
               exit
            end
         end
      end
      if mimeline~='' then do
         Call ProcessMime
      end
      foo=close('IN')
      foo=close('OUT')
   end
   else do
      errmsg='Not a valid file selected'
      Call ErrorMsg
      exit     
   end
   
Return

/**************************************************************************/
/*                   Handle special characters in line                    */
/**************************************************************************/
ProcessMime:

   eqpos=pos('=',mimeline)

   do while eqpos>0
      testchar=substr(mimeline,eqpos+1,2)
      if datatype(testchar,'X') then do
         newchar=x2c(testchar)
         mimeline=delstr(mimeline,eqpos,3)
         mimeline=insert(newchar,mimeline,eqpos-1,1)     
      end 
      eqpos=pos('=',mimeline,eqpos+1)
   end

   if header then do
      foo=writeln('OUT',mimeline)              /* write lines to output  */
   end
   else do
      do while length(mimeline)>choplen /* chop lines to reasonable len  */
         blankpos=lastpos(' ',substr(mimeline,1,choplen))
         if blankpos~=0 then do
            lineout=substr(mimeline,1,blankpos)
            foo=writeln('OUT',lineout)            /* write piece to output  */
            mimeline=substr(mimeline,blankpos+1)  /* remaining portion      */
         end
         else leave
      end
      foo=writeln('OUT',mimeline)           /* write last piece to out*/
   end 

   mimeline=''                                 /* reset to null line     */

Return

/**************************************************************************/
/*                   Handle special characters in headers                 */
/**************************************************************************/
DoQencode:

   qword=''
   qwstart=pos(isomime,upper(linein))
   do while qwstart>0
      qword=substr(linein,qwstart+length(isomime))
      qwend=pos(isoend,qword)
      qword=substr(qword,1,qwend-1)
      qlen=length(isomime)+length(qword)+length(isoend)
      eqpos=pos('=',qword)
      linein=delstr(linein,qwstart,qlen)
      do while eqpos>0
         testchar=substr(qword,eqpos+1,2)
         if datatype(testchar,'X') then do
            newchar=x2c(testchar)
            qword=delstr(qword,eqpos,3)
            qword=insert(newchar,qword,eqpos-1,1)     
         end 
         eqpos=pos('=',qword,eqpos+1)
      end
      qword=translate(qword,' ','_')
      linein=insert(qword,linein,qwstart-1,length(qword))
      qwstart=pos(isomime,upper(linein))
   end
Return

/**************************************************************************/
/*                        Save the mail file back                         */
/**************************************************************************/
SaveFile:     

   fstate=statef(mfile)                           /* file information     */
   fbytes=subword(fstate,2,1)                     /* length of file       */
   fdate=subword(fstate,5,1)                      /* internal date        */
   fmins=subword(fstate,6,1)                      /* minutes since midnite*/
   fticks=subword(fstate,7,1)                     /* ticks past minutes   */
   fcomm=subword(fstate,8)                        /* old comment          */

   fdate=date('E',fdate,'I')                      /* get date in ddmmyy   */
   fdate=translate(fdate,'-','/')                 /* convert / to -       */
   hh=fmins%60                                    /* get hours            */
   hh=right(hh,2,'0')                             /* force to 2 digits    */
   mm=fmins//60                                   /* get minutes          */
   mm=right(mm,2,'0')                             /* force to 2 digits    */
   ss=fticks%50                                   /* seconds              */
   ss=right(ss,2,'0')                             /* force it             */
   ftime=hh||':'||mm||':'||ss                     /* timestamp rebuilt    */

/* Address Command 'COPY 'mfile bakup 'QUIET' */

   Address Command 'COPY 'ofile mfile 'QUIET'
   Address Command 'SETDATE '  mfile fdate ftime
   Address Command 'FILENOTE ' mfile '"'fcomm'"'
   Address Command 'DELETE ' ofile 'QUIET' 
   

Return

/**************************************************************************/
/*                           End of Active Code                           */
/**************************************************************************/
