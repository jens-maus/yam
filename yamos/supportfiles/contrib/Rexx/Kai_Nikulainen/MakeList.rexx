/* $VER:MakeList.rexx  © Kai Nikulainen                                 email: kajun@sci.fi 1.0 (4-Dec-97)
Makes mailinglists for MPDMailPost from the messages in a folder

Script suggested by John Pullicino */

options results
call addlib('rexxreqtools.library',0,-30,0)

defdir='ram:'
defname='templist'
NL='0a'x
tags='rt_pubscrname=YAM'  /* change to the name of Yam's public screen*/
title='Makelist.rexx by Kai Nikulainen'
sea_t='Which messages will I search?'
sea_b=' _All | _Selected | _Cancel '
sel_t='Select address to save in maillist'
sel_b='_Next Page|_Cancel'
yn_b='_Add it!|_No - Go Back!'
error_txt='Could not write to file '
tmpfile='t:templist'

maillist=rtfilerequest(defdir,defname,'Enter name of list',,'rtfi_flags=freqf_save' tags)
address 'YAM'
'GetFolderInfo Max'     /* How many messages are in the folder? */
last=result-1           /* They are numbered from 0..result-1 */
first=0

'GetMailInfo act'       /* Return code rc=0 if a message was selected */
if rc=0 then do
        act=result
        sea=rtezrequest(sea_t,sea_b,title,'rtez_defaultresponse=2' tags)
        if sea=0 then exit
        if sea=2 then do
                first=act
                last=act
                end
        end
else
        act=0
ac=0

do m=first to last
        'SetMail' m
        'GetMailInfo From'
        call AddIt(result)
        'GetMailInfo file'
        call ScanMsgBody(result)
        end

first=1
do until sel=0
        body=ac 'addresses were found:'NL'~~~~~~~~~~~~~~~~~~~~'NL
        buttons=''
        i=first
        lab=0
        do while lab<9 & i<=ac
                body=body || i-first+1 || ') ' || addr.i||NL
                buttons=buttons'_' || i-first+1 ||'|'
                lab=i-first+1
                i=i+1
                end
        buttons=buttons || sel_b

        sel=rtezrequest(body,buttons,title,'rtez_defaultresponse=0' tags)
        if sel>0 then do
                if sel>lab then do
                        first=first+lab
                        if first>ac then first=1
                        end
                else do
                        j=first+sel-1
                        call AddToFile(addr.j)
                        end
                end
        end
address 'YAM' 'SetMail' act
exit

ScanMsgBody:
parse arg fname
if open(6,fname,'r') then do
        r=''
         do until r='' | eof(6)  /* removing these three lines will*/
                 r=readln(6)      /* mean addresses in the headers  */
                 end              /* will also be retrieved         */
        do until eof(6)
                r=translate(readln(6),' ','<>:"',' ')
                do w=1 to words(r)
                        wrd=word(r,w)
                        if pos('@',wrd)>0 then call AddIt(wrd)
                        end /* do w= */
                end /* do until eof */
        call close(6)
        end /* if open */
else
        say "Couldn't open" fname
return

AddIt:
parse arg addy
        addy=strip(addy)        /* Removes leading and trailing spaces, if any */
        foo=upper(addy)         /* just makes things a bit quicker */
        new_addr=1              /* 1=True, 0=False */

        do i=1 to ac            /* ac = address counter */
                if upper(addr.i)=foo then new_addr=0
                end

        if new_addr then do     /* The adress was no in the list yet */
                ac=ac+1         /* increase counter */
                addr.ac=addy    /* and save the address */
                end
return

AddToFile:
parse arg addy

/* There may be name on the line, so lets find the actual address... */
do w=1 to words(addy)   /* Checks every word */
        if pos('@',word(addy,w))>0 then email=translate(word(addy,w),'','"<> ','')
        /* If there is a @ in the word, it's used as an e-mail address, except */
        /* every space, <, > and " is removed from it. */
        end

ad=email

body=' I will add this address to ..'NL maillist NL NL
body=body ad || NL '~~~~~~~~~~~~~~~~~~~~~'
if rtezrequest(body,yn_b,title,'rtez_defaultresponse=0' tags) then do
        if open(tmp,tmpfile,'w') then do

                /* let's copy every address in the old file first */
                if open(inp,maillist,'R') then do
                        do until eof(inp)
                                li=strip(readln(inp))
                                if li~='*' & li~='' then call writeln(tmp,li)
                                end /* do until */
                        call close(inp)
                        end /* if open(inp,maillist,'R') */

                /* Now the new address and an asterisk */
                call writeln(tmp,ad)
                call writeln(tmp,'*')
                call close(tmp)

                /* Then a couple CLI commands */
                address command 
                'copy' tmpfile maillist
                'delete' tmpfile

                end /* if open(tmp,tmpfile,'w')*/
        else /* Couldn't open the tempfile */
                call rtezrequest(error_txt maillist,,title,'rtez_defaultresponse=0')
        end
return

