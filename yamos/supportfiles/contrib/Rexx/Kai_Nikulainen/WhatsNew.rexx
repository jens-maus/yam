/*$VER:WhatsNew.rexx   © Kai Nikulainen                              email:kajun@sci.fi 1.0 (10 Feb 1998)
Calculates how much of the selected message is original, quoted, or signature, and adds a report to the message.
*/
/*Check http://www.sci.fi/~kajun for lots of other scripts! */

options results
quotelen=5		/* how many leftmost characters are searched for > to 
			   detect quotes*/

mstr=' P[2]This message is made up of '
dash=' P[2]~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~'
ostr='    Original text'
qstr='    Quoted Text'
sstr='    Signature'
tempname='t:whatsnew.tmp'

address 'YAM'
'GetMailInfo file'
file=result

if rc>0 then do
	'Request "You must select a message first!" "_Ok"'
	exit
	end

if ~open(msg,file,'r') | ~open(tmp,tempname,'w') then do
	'Request "Can not open files!" "_Exit"'
	exit
	end

/* Lets skip the headers but remember the possible boundary */
boundary=''
do until r='' | eof(msg)
	orig=readln(msg)
	call writeln(tmp,orig)
	r=translate(orig,' ','"')
	if pos('boundary=',r)>0 then boundary='--'||word(r,words(r))
	end

if boundary~='' then do
	do until r=boundary | eof(msg)
		r=readln(msg)
		call writeln(tmp,r)
		end
	r=readln(msg)	/* Should be Content-Type: text/plain line */
	call writeln(tmp,r)
	r=readln(msg)   /* Should be an empty line */
	call writeln(tmp,r)
	end

/* Now we should be at the beginning of the message */
total=0
quoted=0
sig=0
in_sig=0


if boundary='' then boundary=time(s)||date()||'FOOBAR'
do until r=boundary | eof(msg)
	r=readln(msg)
	if ~eof(msg) & r~=boundary then do
		if r='-- ' then in_sig=1
		l=length(r)
		if r=boundary then l=0
		total=total+l
		call writeln(tmp,r)	
		if in_sig then 
			sig=sig+l
		else
			if pos('>',left(r,quotelen))>0 then 
				quoted=quoted+l
		end /* if ~eof */
	end

q=trunc(quoted/total*100+0.5,0)
s=trunc(sig/total*100+0.5,0)
o=100-q-s

call writeln(tmp,mstr)
call writeln(tmp,dash)
call writeln(tmp,'    'o ||'%' || ostr )
call writeln(tmp,'    'q ||'%' || qstr )
call writeln(tmp,'    's ||'%' || sstr )
call writeln(tmp,dash)

if r=boundary then call writeln(tmp,r)	

do while ~eof(msg)
	call writeln(tmp,readln(msg))
	end
	
call close(tmp)
call close(msg)
Address YAM 'request "P[2]       WhatsNew.rexx   P[1]*n*n I have appended the report *n   to the selected message" "_OK"' 
address command /* the following are shell commands */
'copy >nil:' tempname file
'delete >nil:' tempname
exit
	
