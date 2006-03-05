/* $VER: Beheader.rexx © Kai Nikulainen                              email: kajun@sci.fi 1.0 (31-Oct-97)
Removes unwanted headers from messages in the current folder
*/

options results
/* Only the headers listed in the following variable are kept */
keep='From: To: Reply-To: Subject: Date: Organization: Content-type: Message-ID:'
txt='"Shall I strip the P[2]unnecessaryP[1] headers*nfrom all the messages in this folder?"'
but='"_Yes|_No way"'
end_txt='messages have been P[2]BeHeadedP[1] !*n'
bytes='bytes have been saved *nP[2]---------------------------------------P[1]*nUpdate index to see the size change '
end_but='_Update|_Exit'
tmp='T:Beheader.tmp'

address YAM
request txt but
if result=0 then exit
GetFolderInfo max
n=result
c=0
b=0
do m=0 to n-1
        SetMail m
        call StripThem
        if modified then do
                address command 'copy' tmp fname
                c=c+1
                end
        end
address command 'delete >nil:' tmp
'request "'c end_txt || b  bytes'" "'end_but'"'
if result=1 then MailUpdate
exit

StripThem:
        modified=0
        GetMailInfo file
        fname=result
        if ~open(msg,fname,'r') | ~open(new,tmp,'w') then exit
        do until r='' | eof(msg)
                r=readln(msg)
                w=left(r,pos(':',r))
                if pos(w,keep)>0 |r='' then /* this is a header we need to copy */
                        call writeln(new,r)
                else do/* something was removed */
                        modified=1
                        b=b+length(r)+1
                        end
                end
        if modified then 
                do until eof(msg)
                        r=readln(msg)           
                        if ~eof(msg) then call writeln(new,r)
                        end
        call close(msg)
        call close(new)
return
