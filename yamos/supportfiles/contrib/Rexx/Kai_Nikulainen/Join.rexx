/* $VER: Join.rexx 1.3 - 17-Oct-97 by Kai.Nikulainen@sci.fi
** Joins several messages into a file and uudecodes it.
**
** Send all comments, bug reports, suggestions and X-mas cards to kajun@sci.fi
*/

options results
call addlib('rexxsupport.library',0,-30)
call addlib('rexxreqtools.library',0,-30)

uudecode='c:uudecode >nil:'	/* Change here the program you use for decoding. */
defdir='ram:'			/* Default for dir-requester */
tempname='t:join.tmp'		/* Messages will be joined into this file */
				/* If the file is huge, don't use ram:    */
tags='rt_pubscrname=YAMSCREEN'  /* Change here the name of the screen YAM runs on*/
maxlines=20			/* How many lines fit into one requester? */

/* Reqtools gadget texts */
strbody='Enter string pattern for subjects.  Use * in place of part numbers.'
ordbody='I found these parts in this order.  Select a part if it should be moved:'
newbody='Before which part it should be moved?'
  buts3='_All in order|_Exit'
  buts4='_Cancel'
  title='Join.rexx 1.3 © kajun@sci.fi'


mc=0 
used=0
NL='0a'x

address 'YAM'
'GetMailInfo From'
server=result
'GetMailInfo Subject'
subj=result
'GetMailInfo Active'
active=result
'GetFolderInfo Max'
n=result
pattern=rtgetstring('*'subj'*',strbody,title,,tags,button)
if button=0 then exit


/* 
** First scan all messages in the folder.
** Search for messages with same sender and which match the pattern
*/

do m=0 to n-1
    'SetMail' m
    'GetMailInfo Subject'
    if match(pattern,result) then do
       	call AddMsg(result)
	part.mc=m
	end

end /* do m=1 to n */
'SetMail' active  /* Go back to the originally selected message */
/*
** Then sort the messages.  Hopefully part 10 comes after 9 and not after 1....
*/

call SortMessages

if mc<=maxlines & mc>1 then do
do until sel=mc+1  
  call ListMsgs
  def=mc+1
  sel=rtezrequest(ordbody || msgs,buts || buts3,title,'rtez_defaultresponse='def tags)
  if sel=0 then exit
  if sel<def then do
  	new=rtezrequest(newbody || msgs,buts || buts4,title,'rtez_defaultresponse=0' tags)
	if new~=sel & new~=sel+1 & new>0 then do
		n=1
		do o=1 to new-1
			if o~=sel then do
				su.n=subject.o
				fi.n=filename.o
				pa.n=part.o
				n=n+1
				end /* if */
			end /* do o */
		su.n=subject.sel
		fi.n=filename.sel
		pa.n=part.sel
		n=n+1
		do o=new to mc
			if o~=sel then do
				su.n=subject.o
				fi.n=filename.o
				pa.n=part.o
				n=n+1
				end /* if */
			end /* do o=new */
		do o=1 to mc
			filename.o=fi.o
			subject.o=su.o
			part.o=pa.o
			end /* do o=1 to mc */
		end /* if new~=sel */
  	end
  end
end

/*
** Lets write the messages into one file
*/
outdir=rtfilerequest(defdir,'','Select output directory',,tags 'rtfi_flags=freqf_nofiles',fsel)
if fsel=0 then exit

call pragma('D',outdir)

if ~open(out,tempname,'w') then do
	'Request "Can not write tempfile!" "_Exit"'
	exit
	end

do i=1 to mc
        if ~open(inp,filename.i,'r') then do
		'Request "Can not read part*n'subject.i'" "Ok"'
		'SetMail' active  /* Go back to the originally selected message */ 
		exit
		end
	do until eof(inp) | r='' /* Skip headers */
		r=readln(inp)
		end
	do until eof(inp)
		r=readln(inp)
		if r~='' then call writeln(out,r)
		end
	call close(inp)
	'SetMail' part.i
	'MailDelete'
        end /* do i=1 to mc */
call close(out)

address command 
uudecode tempname
'delete >nil:' tempname

address 'YAM'
'SetMail' active  /* Go back to the originally selected message */
exit	/* It's the end of the script as we know it... */


ListMsgs:
  msgs=''
  buts=''
  do i=1 to mc
	msgs=msgs NL || right(i || ')',3) subject.i
	if i<10 then
		etu='_'
	else
		etu=''
	buts=buts || etu || i || '|'
	end
return

FindStart:
  found_it=0
  if used=0 then do
  /* 
  ** When called the first time, this branch checks which delimiters are used 
  ** Following calls use else branch
  */
     do until eof(1) | found_it
         rivi=readln(1)
         do d=1 to delimiters
             if pos(startline.d,rivi)=1 then do
	          found_it=1
	          used=d
		  end /* if */
              end /* do d=1 */
         end /*do until */
     if used=3 then call writeln(tmp,rivi)  /* Save uuencode first line */
     end /* ifused=0 */
  else do
     do until eof(1) | found_it
         rivi=readln(1)
         found_it=pos(startline.used,rivi)
         end /*do until */  
     end /* else do */
return found_it

AddMsg:
/*
** This changes the subject's (1/n) to (01/n) to allow sorting
*/
  parse arg s
  mc=mc+1
  bra=lastpos('(',s)
  if bra>0 then do
      slash=pos('/',s,bra)
      if slash=0 then do
          slash=lastpos('/',s)
          if slash>0 then bra=lastpos(' ',s,slash)
	  end
      end
  else do
      slash=lastpos('/',s)
      if slash>0 then bra=lastpos(' ',s,slash)
      end
  if slash-bra=2 then s=left(s,bra)'00'substr(s,bra+1)
  if slash-bra=3 then s=left(s,bra)'0'substr(s,bra+1)
  subject.mc=s
  'GetMailInfo File'
  filename.mc=result
return

SortMessages:
/* 
** A simple algorithm is fastest with relatively few items.
** There should be no need for something fancy like quicksort :-) 
*/
  do i=2 to mc
     do j=1 to i-1
        if upper(subject.j)>upper(subject.i) then do/* let's swap stuff... */
		temp=subject.j
		subject.j=subject.i
		subject.i=temp
		temp=filename.j
		filename.j=filename.i
		filename.i=temp
		temp=part.j
		part.j=part.i
		part.i=temp
		end /* if */
     end /* do j */
  end /* do i */
return  /* Everything should be in order now... */

Match: procedure
parse arg pat,str
	res=0
	pat=upper(pat)
	str=upper(str)
	p1=pos('*',pat)
	if p1=0 then
		res=(pat=str)
	else do
		alku=left(pat,p1-1)	/* chars before first * */
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

