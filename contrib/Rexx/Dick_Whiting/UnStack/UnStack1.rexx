/******************************************************************************/
/*                                                                            */
/*                           UnStackMail.rexx                                 */
/*                 Copyright ©1999 by Dick Whiting                            */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*  Should take a single file of email that have just been appended together  */
/*  and split them into individual mail files.                                */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*  Run from a CLI/DOPUS/etc.                                                 */
/*                                                                            */
/*  rx UnStack1.rexx filein/A output-directory/A                              */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* The logic of this ASSUMES that a new mail will start with one of the       */
/* following lines:                                                           */
/*                                                                            */
/*      Return-Path:                                                          */
/*      Received:                                                             */
/*                                                                            */
/* If this is NOT true, then some mail files will not be processed properly.  */
/*                                                                            */
/* Files are saved in the output-directory with names based on the date       */
/* the script is run and should be reasonable for Yam.                        */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*                           USAGE INSTRUCTIONS                               */
/*  1) Use Yam to create new folders, one for each of the input files.        */
/*     e.g. Yam:junk1, Yam:junk2, ....                                        */
/*                                                                            */
/*  2) Run the script from a CLI against the first input file with the        */
/*  output-directory being your first new Yam folder.                         */
/*                                                                            */
/*  3) In YAM go to that directory and do an Update Index                     */
/*                                                                            */
/*  4) If all goes well, you can repeat steps 2 & 3 for each additional       */
/*  input-file output-directory pair.                                         */
/*                                                                            */
/*  WARNING!!! make sure you DON'T run with the same OUTPUT directory twice.  */
/*  Existing files will be replaced.                                          */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/* WARNING!! use a DIFFERENT output-directory for each input file.            */
/* There's NO checking for conflicting output filenames.                      */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*               Address Bug Reports or Comments to:                          */
/*                Dick Whiting <dwhiting@europa.com>                          */
/*                            09 Sep 1999                                     */
/*                                                                            */
/******************************************************************************/
/*
$VER: 1.0 Copyright ©1999 by Dick Whiting
$AUTHOR: Dick Whiting
$DESCRIPTION: Split appended file of mail into individual files.
*/

options results

if ~show('L','rexxsupport.library') then do
   addlib('rexxsupport.library',0,-30)
end

filein=''   
dirout=''

TRUE=1
FALSE=0

body=TRUE                                   /* flag for doing msg body    */

fdate=date('I')                             /* Yam style filename part    */
fdate=fdate-20                              /* start names 20 days ago    */
fdate=right(fdate,5,'0')                    /* starting filename portion  */

fsuf=0                                      /* suffix counter for files   */

parse arg filein dirout rest

if filein='' | dirout='' then do
   say "  Missing arguments"
   say "  Format: UnStackMail filein/A output-directory/A"
   exit 20
end

fstat=statef(filein)
parse var fstat ftype rest

if ftype~='FILE' then do
   say " " filein "is not a FILE"
   exit 20
end

if (right(dirout,1)~='/' & right(dirout,1)~=':') then dirout=dirout||'/' 

fstat=statef(dirout)
parse var fstat ftype rest

if ftype~='DIR' then do
   say " " dirout "is not a DIRECTORY"
   exit 20
end

say ""
say "Starting processing...this could take awhile...."
say ""

/******************************************************************************/
/*  Read the input file looking for Return-Path: and Received: starts         */
/******************************************************************************/

if open('IN',filein,'R') then do
   do until eof('IN')
      linein=readln('IN')
      select
         when ~body & linein='' then do
            body=TRUE
         end
         when body & upper(substr(linein,1,13))='RETURN-PATH: ' then do
            Call OpenNew
            body=FALSE
         end
         when body & upper(substr(linein,1,10))='RECEIVED: ' then do
            Call OpenNew
            body=FALSE
         end
         otherwise nop
      end
      foo=writeln('OUT',linein)
   end
end
else do
   say "  Unable to open" filein "for input"
   exit 20
end

foo=close('OUT')                            /* Close previous output file */

exit 0

/**************************************************************************/
/*                  Construct output filename and open it                 */
/**************************************************************************/
OpenNew:

   fsuf=fsuf+1                              /* bump suffix counter        */

   if fsuf>999 then do                      /* more than 999 mail         */
      fdate=fdate+1
      fdate=right(fdate,5,'0')
      fsuf=1
   end
 
   fsuf=right(fsuf,3,'0')                   /* 3 character file suffix    */
   
   fileout=dirout||fdate||'.'||fsuf         /* build full filename        */

   foo=close('OUT')                         /* Close previous output file */

   if ~open('OUT',fileout,'W') then do
      say "  Unable to open" fileout "for output"
      exit 20
   end

return

