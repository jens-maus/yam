/* $VER: CountryCode.rexx 1.0 - Kai Nikulainen (25-Jun-97)
** Shows country codes of addresses in the current message with 
** CountryCodes v1.0 by Paul Kolenbrander
** CountryCodes can be found in Aminet comm/www/CountryCodes.lha
** 
** YAM script written by Kai Nikulainen.  If you have comments about the script,
** send mail to knikulai@utu.fi
*/
options results

/* CHANGE the correct path to the following line */
coco_prg='Packed:Datacomm/CountryCodes/CountryCodes' 

 gtxt='Select address:*n'	/* Change these to modify the requester */
 gscr='_Scroll'			/* _ means keyboard shortcut */
 gbut='_Exit'

 hdrs=4		/* Search e-mail addresses from these headers */
hdr.1='To:'
hdr.2='From:'
hdr.3='Cc:'
hdr.4='Bcc:'

adrs=0

address 'YAM'
'GetMailInfo File'
call GetAddrs(result)

cur=1

if pos(' COCO',show(p))=0 then address command 'run >nil:' coco_prg
l=show(p)
p=pos('COCO',l)
coco=substr(l,p,5)

do while 1
	newcur=CreateRequester(cur)
	address 'YAM' 'request "'txt'" "'but'"'
	if result=0 then exit
	if result=ScrollButton then
		cur=newcur
	else do
		index=cur+result-1
		p=lastpos('.',adr.index)
		suf=substr(adr.index,p)
		address value coco 
	 	'WINDOW FRONT'
		'LOCATE' suf
		end
	end
exit

CreateRequester:
parse arg c
	txt=gtxt
	but=''
	b=1
	old_c=c
	do until c>adrs | b=8 | 210<(length(txt)+length(gscr)+length(gbut)+length(but)) 
		txt=txt || b')' adr.c || '*n'
		but=but || '_' || b || '|'
		c=c+1
		b=b+1
		end
	if old_c>1 | c<adrs then but=but || gscr'|'
	but=but || gbut		
	ScrollButton=b
	if c>adrs then c=1
return c

GetAddrs: procedure expose hdrs hdr. adrs adr.
parse arg fn
	call open(1,fn,'r')
	do until eof(1) | r=''
		r=translate(readln(1),' ','<>,'||'09'x)
		w1=word(r,1)
		if right(w1,1)=':' then h=upper(w1)
		do i=1 to hdrs
			if upper(hdr.i)=h then call Extract(r)
			end
		end /* until */
	call close(1)
return

Extract: procedure expose adrs adr.
parse arg line
	do i=1 to words(line)
		w=word(line,i)
		if pos('@',w)>0 then call AddIt(w)
		end
return

AddIt: procedure expose adrs adr.
parse arg a
	found_it=0
	do i=1 to adrs
		if upper(a)=upper(adr.i) then found_it=1
		end
	if ~found_it then do
		adrs=adrs+1
		adr.adrs=a
		end
return
