/* $VER:Split'n'Mail.rexx  © Kai Nikulainen                            email: kajun@sci.fi 1.2 (24-Jun-97)
Script for splitting and posting a long ASCII file.

  Requirements:
    - YAM 
    - reqtools.library
    - rexxsupport.library
    - rexxreqtools.library
    - a really long ascii file you want to mail

  Operation:
    - The script splits the file automatically OR when a line consisting only
      '.break' (without quotes) is encountered.
    - By changing the value of use_separators, you can decide if the parts are
      enclosed between the following lines:
      BEGIN PART 6 --- *** --- *** --- *** ---
      END PART 6 --- *** --- *** --- *** ---
  
  Send bug reports, comments and hate mail to kajun@sci.fi

  If you want to get the list of my AREXX-scripts or their latest versions, 
  send mail to kajun@sci.fi with subject 'request' and words 'help' and
  'allfiles' in the message body, both on it's own line.
*/

options results
breakline='.break'      /* You can change this to anything you like */
use_separators='yes'    /* This value should be 'yes' or 'no' */
tmp='t:'                /* Temporary files are stored here.  You might want to
                           change this if you are low on memory or the file is 
                           very large. */

title="Split'n'Mail by knikulai@utu.fi"
butts='_Ok|_Cancel'     /* :-) I know it's childish... :-) */
tags='rt_pubscrname=YAMSCREEN'  /* Change here the name of the screen YAM runs */
call addlib('rexxreqtools.library',0,-30)
call addlib('rexxsupport.library',0,-30)

email=rtgetstring('','Enter recipients e-mail address',title,butts,tags,selection)
if selection=0 then call ErrorMsg("You must enter an address!")

filename=rtfilerequest('','','Select file to post',,tags)
if filename='' | ~exists(filename) then call ErrorMsg("File doesn't exist!")

maxl=rtgetlong(400,'How many lines per part?',title,butts,tags,selection)
if selection=0 | maxl<50 then call ErrorMsg("Too few lines per part!")

subj=rtgetstring(filename,'Enter subject of post',title,butts,tags,selection)
if selection=0 then call ErrorMsg("You must enter a subject!")

ok=open(infile,filename,'R')
if ~ok then call ErrorMsg("Can't open file!")

fc=1  /* part counter */

do while ~eof(infile)
  ok=open(outfile,tmp || fc,'W')
  if ~ok then call ErrorMsg("Could not open file for part "llfc)
  if upper(use_separators)='YES' then call writeln(outfile,'BEGIN PART '||fc||' --- *** --- *** --- *** ---')
  lc=0 /* line counter */
  do until line=breakline | lc=maxl | eof(infile)
    line=readln(infile)
    lc=lc+1
    if line~=breakline then do
        n=writeln(outfile,line)
        if n<=length(line) then call ErrorMsg("Can't write to file!")
        end
  end /* until line=breakline l lc=maxl */
  if upper(use_separators)='YES' then call writeln(outfile,'END PART '||fc||' --- *** --- *** --- *** ---')
  call close(outfile)
  if ~eof(infile) then fc=fc+1
end /* until eof(infile) */

address 'YAM'
do i=1 to fc
  'MailWrite'
  'WriteMailTo "' || email ||'"'
  'WriteSubject "'|| subj || ' (' || i || '/' || fc || ')"'
  'WriteLetter "' || tmp || i || '"'
  'WriteQueue'
  call delete(tmp || i)
end /* i=1 to fc */

call TheEnd /* Goodbye! */

ErrorMsg:
        parse arg s
        call rtezrequest(s,"_Exit","Split'n'Mail error",tags)
TheEnd:
        call remlib('rexxreqtools.library')
        call remlib('rexxsupport.library')
        exit /* All good things must eventually come to an end :-) */
