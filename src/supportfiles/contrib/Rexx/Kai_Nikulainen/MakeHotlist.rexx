/*$VER:MakeHotlist.rexx © Kai Nikulainen                                email: kajun@sci.fi 1.2 (03-Jun-97)
Grabs URLs from messages in the current folder and creates a hotlist which it sends to a browser.


   *IMPORTANT* You need to change the variable browser to a correct
   value or the script will not be able to run your browser.

   All adresses which end with a character in variable dup_chars will be
   duplicated and displayed also without that character.

   This script requires rexxreqtools and reqtools libraries, if the
   filerequester is used.

   Send bug reports, comments and spare bodyparts to kajun@sci.fi
*/
options results

/* CHANGE THE FOLLOWING LINE */
browser='Work:IBrowse/IBrowse'          /* Start this program if no browser is running */
hname='YAM:HotList.html'                /* Full path and name of hotlist */
tags='rt_pubscrname=YAMSCREEN'  /* Change here the name of the screen YAM runs */

/* If you wish to select the name of the hotlist, uncomment next three lines*/
/*
call addlib('rexxreqtools.library',0,-30,0)
hname=rtfilerequest('YAM:','HotList.html','Enter name of the hotlist',,'rtfi_flags=freqf_save' tags)
if hname='' then exit
*/

dup_chars='.,!?"*%&)'   /* If one of these ends the URL, it will be duplicated */
not_found='Sorry,no URLs were found in any of the read messages!'
uc=0                    /* URL-counter */

call open(hotlist,hname,'W')

address 'YAM'
'Request "Do you want to search for URLs from" "_All messages|_This message|_Quit"'
if result=0 then exit
OnlyOne=(result=2)

writeln(hotlist,'<html><head><title>URLs from YAM</title></head>')
'GetFolderInfo Name'
writeln(hotlist,'<body><center><h3>URLs from messages in folder' result'</h3></center>')
writeln(hotlist,'<center>An URL is only displayed once, even if it appears in several messages.</center>')
writeln(hotlist,'<hr><ul>')

'GetFolderInfo Max'
loppu=result-1
alku=0
'GetMailInfo Act'
act=result

if OnlyOne then do
        alku=act
        loppu=act
        end

do viesti=alku to loppu
    'SetMail' viesti
    'GetMailInfo File'
    fname=result
    'GetMailInfo Subject'
    sub=result
    'GetMailInfo Status'
       if open(msg,fname,'R') then do
           do until eof(msg)
             rivi=translate(readln(msg),' ','"')
             rivi=rivi || '  '                /* Just making sure there is a space at the end */
             do while (pos('HTTPS://',upper(rivi))>0 | pos('GOPHER://',upper(rivi))>0 | pos('HTTP://',upper(rivi))>0 | pos('FTP://',upper(rivi))>0)
                /* There might be several URLs on one line */
                b=pos('HTTP://',upper(rivi))          /* beginning */
                if b=0 then b=pos('FTP://',upper(rivi))
                if b=0 then b=pos('GOPHER://',upper(rivi))
                if b=0 then b=pos('HTTPS://',upper(rivi))
                e=pos(' ',rivi,b)                     /* space ends the URL */
                if e-pos('//',rivi,b)>2 then do       /* only save if URL has chars in it */
                    call AddURL(substr(rivi,b,e-b))   /* save URL */
                    spare=url.uc
                    do while pos(right(spare,1),dup_chars)>0
                        /* Add another URL without the last character */
                        spare=left(url.uc,length(url.uc)-1)
                        call AddURL(spare)
                        end /* do while pos */
                end /*if e-pos('//',rivi,b)>2*/
                rivi=right(rivi,length(rivi)-e)
             end /* do while (pos('GOPHER://',upper(rivi))> */
           end /* do until eof(msg) */
         close(msg)
      end /* if open(msg,fname,'R') then do */

end /* do viesti=0 to n-1 */
writeln(hotlist,'</ul><hr><a href="mailto:knikulai@utu.fi">Send praise, bug reports and pictures of Scully to knikulai@utu.fi</a></body></html>')
call close(hotlist)

if uc=0 then
   'request "'||not_found||'" "_Ok"'
else do
   lst=show('P') || '  '
   hlst='file:///'hname
   no_browser=1
   if pos('MINDWALKER',lst)>0 then do
      address 'MINDWALKER' 'OpenURL' hlst
      no_browser=0
      end
   if pos('VOYAGER',lst)>0 then do
      address 'VOYAGER' 'OpenURL' hlst 
      no_browser=0
      end
   if pos('IBROWSE',lst)>0 then do
      address 'IBROWSE' 'GotoURL' hlst
      no_browser=0
      end
   if pos('AWEB',lst)>0 then do
        address value substr(lst,pos('AWEB.',lst),6)
        'Open file://localhost/' || hname
        no_browser=0
        end
   if no_browser then do
        'Request "You do not have a browser running.  The *nhotlist has been saved toP[3]' hname'P[1]*nWhat should I do?" "_Run browser|_End script"'
        if result=0 then exit
        address command 'run <>nil:' browser hlst
        end
   else
        'Request "The hotlist has been sent to your browser." "_Ok"'
end /* else */
'SetMail' act

exit

AddURL:
  parse arg u
  do i=1 to uc
      if url.i=u then return
      end
  uc=uc+1
  url.uc=u
  if sub~='' then do /* First write the subject of the message */
        writeln(hotlist,'</ul><p><p><strong>'|| sub ||'</strong><ul>')
        sub=''
        end
  writeln(hotlist,'<li><a href="'|| u ||'">'|| u ||'</a>')
return
