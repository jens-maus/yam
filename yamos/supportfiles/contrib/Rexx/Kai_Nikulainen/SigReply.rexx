/* $VER:SigReply.rexx   © Kai Nikulainen                                email: kajun@sci.fi  1.1 (20-Jul-97)
Allows you to choose between From: and Reply to: when replying, and to use different signatures for different domains.
*/

options results
call addlib('rexxsupport.library',0,-30,0)

origsig='YAM:orig.sig'  /* The signature will be reset to this file */
sigfile='YAM:.signature'
datfile='YAM:sigs.txt'  /* Domain dependant signatures.  File format: */
                        /* @ .fi .se                    */
                        /* Signature lines for          */
                        /* .fi and .se                  */
                        /* @ .com .net                  */
                        /* Strange domain signature     */
                        /* @                            */
                        /* Default signature            */

                        /* The default sig must be last! */
address 'YAM'

'GetMailInfo File'      /* Filename */
fname=result
if rc>0 then do
        'Request "Select a message before starting the script!" "_Ok"'
        exit
        end

if ~exists(datfile) then do     /* Create a sample if necessary */
        call CreateSample
        exit
        end

if ~open(1,datfile,'r') then do
        'Request "Could not open' datfile '" "_Ok"'
        exit
        end

count=0

do until eof(1)                 /* Read whole datafile */
        r=readln(1)
        if left(r,1)='@' then do
                count=count+1
                dom.count=upper(strip(substr(r,2)))
                sig.count=''
                end
        else do
                if count=0 then do
                        'Request "Illegal format in' datfile '" "_Ok"'
                        exit
                        end
                sig.count=sig.count || r || '0a'x
                end
        end
call close(1)


replyto=''
   from=''
call open(1,fname,'r')
do until r='' | eof(1) | (from~='' & replyto~='')
        r=translate(readln(1),' ','<>'||'09'x,' ')
        do k=1 to words(r)
                if pos('@',word(r,k))>0 then w=strip(word(r,k))
                end
        if upper(word(r,1))='FROM:' then 
                from=w
        if upper(word(r,1))='REPLY-TO:' then 
                replyto=w
        end
call close(1)

if replyto~='' then
        email=replyto
else
        email=from
if (from~='' & replyto~='') & from~=replyto then do
        txt='Which address should I use:*n*n    From:' from '*nReply-To:' replyto
        'Request "'txt'" "_From|_Reply-To|_Exit"'
        if result=0 then exit
        if result=1 then email=from
        end

country=upper(substr(email,lastpos('.',email)))

c=0
do i=1 to count-1
        if pos(country,dom.i)>0 then c=i
        end
if c=0 then c=count

if ~open(1,sigfile,'w') then do
        'Request "Can not write to' sigfile'" "_Ok"'
        exit
        end

call writeln(1,sig.c)
call close(1)

'MailReply'
'WriteMailTo' email
address command 'copy >nil:' origsig sigfile
exit

CreateSample:
        if ~open(2,datfile,'w') then do
                'Request "Could not write' datfile'" "_Ok"'
                exit
                end
        call writeln(2,'@ .fi .se .no')
        call writeln(2,'Nordic signature')
        call writeln(2,'Signature can be several lines long')
        call writeln(2,'@ .gov')
        call writeln(2,'Bureaucrats rule!')
        call writeln(2,'@ .com')
        call writeln(2,'Everything is so commercial these days...')
        call writeln(2,'@' || '0a'x || 'Default signature must be last!')
        call close(2)
        'Request "Example file' datfile 'was created.*nNow edit it to your own liking." "_Ok"'
return
