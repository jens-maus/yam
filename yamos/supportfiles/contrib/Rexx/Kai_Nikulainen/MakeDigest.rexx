/* $VER:MakeDigest.rexx  © Kai Nikulainen                              email: kajun@sci.fi 1.2 (04-Sep-97 )
Joins several messages into one using a given subject pattern or range of dates.
*/

/* Q: What does it do?
** A: Joins several messages into one which can for example be searched 
**    by YAMtools.  Anyway it makes the folder look cleaner...
**
** Q: Which messages are added?  
** A1: All which match given subject pattern.  * can be used as a wildcard.
** A2: All message which fit into given date range
**
** Q: What happened to the attachments?
** A: They are still there, YAM just doesn't see them anymore.  You need to
**    use some other program, for example mpack to extract them.
**
** Q: How many gorillas does it take to screw in a light bulb?
** A: Only one, but it sure takes a shitload of light bulbs!
**
** If you have any problems, mail me at knikulai@utu.fi
*/
options results
call addlib('rexxreqtools.library',0,-30,0)

addrheader='To'         /* Must be 'from' or 'to'.  Decides which field is used */
                        /* for the default sender address for the digest */ 
defper=14               /* Default period for date selection*/
headers=7               /* ONLY these headers are copied to the digest */
hdr.1='Reply-To:'       /* If you add or remove headers, remember to   */
hdr.2='To:'             /* change variable headers to a correct value! */
hdr.3='Date:'
hdr.4='Subject:'
hdr.5='From:'
hdr.6='Content-Type:'
hdr.7='Sender:'

months='Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec'
temp='t:digest.tmp'     /* messages */
indx='t:index.tmp'      /* digest headers and message subjects */
timezone='+0000'
receiver='be or not to be'
separator='----*---- ----*---- ----*---- ----*---- ----*---- ----*---- ----*---- '
/* Separator is added between messages.  If it's empty, nothing is written. */

title='The following messages are included in this digest:' || '0a'x

join='c:join >nil:'
delete='c:delete >nil:'

/* Header selection gadget */
  htxt='These headers are currently copied to the digest.' || '0a'x
  htxt=htxt || 'Select a header if you do *not* want it copied'
htitle='Modify message headers'
 hbuts='_Ok|_Exit'

/* These texts are for the subject pattern entry field */
  ptxt='Enter search pattern for subjects?'
ptitle='Select messages which will be combined'
 pbuts='_Ok|_Exit script'

/* Subject entry gadget */
  stxt='Enter subject for the digest?'
stitle='Message subject'
 sbuts='_Ok|_Exit script'

/* Dates */
  dtxt='What should be the first date to join?'
  etxt='How many days of messages should be joined?'
dtitle='Set selection criteria'
 dbuts='_Ok|_Exit script' 

/* Search criteria selection */
  qtxt='How do you want to select messages?'
qtitle='Selection criteria'
 qbuts='By _Date|By _Subject|_Exit script'

/* Index selection*/
  itxt='Do you want to include an index of subjects?'
ititle='Index selection'
 ibuts='_Yes|_No'

/* Range confirmation */
  ctxt='Starting to combine messages from' || '0a'x
ctitle='Confirm range'
 cbuts='_Correct|_Wrong range|_Exit script'

/* From header */
  ftxt='Who should the sender of the digest?'
ftitle='Set digest header'
 fbuts='_Ok|_Exit script'

/* this is common for all reqtools requesters */
tags='rt_pubscrname=YAMSCREEN'


/* If you change anything below this line, there could be trouble...*/

if ~open(1,temp,'w') then do
        'Request "Could not write' temp'" "_Quit"'
        exit
        end

if ~open(3,indx,'w') then do
        'Request "Could not write' indx'" "_Quit"'
        exit
        end


address 'YAM'
'GetFolderInfo Name'
subj=result 'digest' 

AddHeaders=1

'GetFolderInfo Max'     /* How many messages are there? */
n=result

'GetMailInfo Subject'
str=result
if left(upper(str),3)='RE:' then str=substr(str,4)

'GetMailInfo Active'
act=result
if rc>0 then do
        str='*'
        curname=''
        end
else do
        str='*'str'*'
        'GetMailInfo file'
        curname=result
        end
str=strip(str)

maximum=0
'SetMail 0' 
'GetMailInfo file'
name=result
if curname='' then curname=name
p1=lastpos(':',name)
p2=lastpos('/',name)
p=max(p1,p2)+1

cri=rtezrequest(qtxt,qbuts,qtitle,tags)
if cri=0 then exit

AddIndex=rtezrequest(itxt,ibuts,ititle,tags 'rtez_defaultresponse=1')

if cri=2 then do
        pattern=upper(rtgetstring(str,ptxt,ptitle,pbuts,tags))
        if pattern='' then exit
        subj=subj date()
        end
else do
        firstd=GetDate(curname)
        do until sel=1
                ds=rtgetstring(date('n',firstd),dtxt,dtitle,dbuts,tags)
                if ds='' then exit
                parse var ds day month year
                mo=right('0' || 1+(pos(month,months)-1)/4,2)
                da=right('0' || day,2)
                ye=right('19' || year,4)
                firstd=date('i',ye || mo || da,'S')
                delta=rtgetstring(defper,etxt,dtitle,dbuts,tags)
                if delta='' then exit
                if datatype(delta)~='NUM' then delta=0
                lastd=firstd+delta-1
                fstr=date('n',firstd)
                lstr=date('n',lastd)
                sel=rtezrequest(ctxt || fstr 'to' lstr,cbuts,ctitle,tags)
                if sel=0 then exit
                end
        subj=subj ' ' fstr '-' lstr
        end

subj=rtgetstring(subj,stxt,stitle,sbuts,tags)
if subj='' then exit
call writeln(3,'Subject:' subj)
call writeln(3,'To:' receiver)

click=0
do until click=headers+1 | headers<1
        bu='' /* buttons */
        bo='' /* body */
        def=headers+1
        do i=1 to headers
                if i<10 then 
                        prefix='_'
                else
                        prefix=''
                bu=bu || prefix || i || '|'
                bo=bo || '0a'x || right( i || ')',3) ' ' hdr.i
                end
        click=rtezrequest(htxt || bo,bu || hbuts,htitle,tags 'rtez_defaultresponse='def)
        if click=0 then exit
        if click<def then do
                do ii=click to headers
                        jj=ii+1
                        hdr.ii=hdr.jj
                        end
                headers=headers-1
                click=0
                end
        end
        
do m=0 to n-1
        'SetMail' m
        'GetMailInfo file'
        fname=result
        d=GetDate(fname)
        if cri=2 then do /* get the subject */
                'GetMailInfo Subject'
                subj=result
                end
        if maximum<substr(fname,p) then maximum=substr(fname,p)
        if (match(pattern,subj) & cri=2) | (d>=firstd & d<=lastd & cri=1) then do
                if AddHeaders then do
                        'GetMailInfo' addrheader
                        fr=rtgetstring(result,ftxt,ftitle,fbuts,tags)
                        if fr='' then exit
                        call writeln(3,'From:' fr)
                        call writeln(3,RememberDate)
                        call writeln(3,'')
                        if AddIndex then call writeln(3,title)
                        AddHeaders=0
                        end
                call CopyMsg(fname)
                if AddIndex then do
                        'GetMailInfo Subject'
                        call writeln(3,result)
                        end
                if separator~='' then call writeln(1,separator)
                'MailDelete'
                end
        end

q=lastpos('.',maximum)
ext=right('00'substr(maximum,q+1)+1,3)
newname=left(fname,p-1) || left(maximum,q) || ext
call close(1)
if AddIndex then do
        if separator~='' then call writeln(3,separator)
        call writeln(3,'')
        end
call close(3)
        
address command join indx temp 'to' newname
address command delete temp indx

'MailUpdate'
'SetMail' act
exit    /* Welcome to the edge of the world */

GetDate: 
parse arg fn
        res=0
        r=''
        call open(2,fn,'r')
        do until eof(2) | r='' | word(r,1)='Date:'
                r=translate(readln(2),' ','09'x)
                end
        call close(2)
        if word(r,1)='Date:' then do
                RememberDate=r
                if pos(',',r)=0 then
                        parse var r 'Date:' day month year .
                else
                        parse var r 'Date:' wd',' day month year .
                mn=right('0' || 1+(pos(month,months)-1)/4,2)
                da=right('0' || day,2)
                ye=right('19' || year,4)
                res=date('i',ye || mn || da,'S')
                end
return res

CopyMsg:
parse arg fn
        call open(2,fn,'r')
        do until r=''
                r=readln(2)
                w=word(translate(r,' ','09'x),1)
                do i=1 to headers
                        if w=hdr.i then call writeln(1,r)
                        end
                end
        do until eof(2)
                call writeln(1,r)
                r=readln(2)
                end
        call close(2)
return

Match: procedure
parse arg pat,str
        res=0
        pat=upper(pat)
        str=upper(str)
        p1=pos('*',pat)
        if p1=0 then
                res=(pat=str)
        else do
                alku=left(pat,p1-1)     /* chars before first * */
                ale=length(alku)
                p2=lastpos('*',pat)
                if left(str,ale)~=alku then
                        res=0
                else 
                        if p1=length(pat) then 
                                res=1
                        else do
                                loppu=substr(pat,p1+1)
                                p2=pos('*',loppu)
                                if p2=0 then
                                        res=(right(str,length(loppu))=loppu)
                                else do
                                        seur=left(loppu,p2-1)
                                        i=ale
                                        do while pos(seur,str,i+1)>0
                                                i=pos(seur,str,i+1)
                                                res=(res | Match(loppu,substr(str,i)))
                                                end
                                        end
                                end /* else do */       
                        end
return res
