/* $VER: Fileserver.rexx by Kai Nikulainen - 11-Apr-97 v2.2
** Check http://www.utu.fi/~knikulai/ARexx.html for more useful scripts! 
** 
**               CD - change current directory. CD without arguments returns to root
**         SEND,GET - send the requested file
** ALLFILES,LISTALL - send a listing of all files in the directory
**   DIR,INDEX,LIST - send a listing of a directory.  Path is optional.
**   INFO,SHOW,VIEW - display file size, filenote and archive contents
**             QUIT - further lines in message body are ignored
**             HELP - send the helpfile
**
** If you start the script from shell by typing 'rx fileserver2.rexx index' and the
** variable allfiles contains a valid filename it will create an index file which 
** can be used to speed up the ALLFILES command.  Please note that if allfiles
** variable is defined, the index sent may be out of date.
**
** If you have any problems or suggestions, mail me at knikulai@utu.fi
**
** This script would not have been written without a request from
** Jon Jeffels <tiger@zetnet.co.uk> 
** 
*/
options results
parse arg opt1
call addlib('rexxsupport.library',0,-30,0)

/* 
** Change these variables to fit your system
*/
folder=0				/* 0=Incoming, 1=Outgoing, etc..             */
					/* Change this to 1 for testing		     */
maxline=75				/* Line length for directory listings        */
dirle=8					/* How many letters should be reserved for   */
					/* dir names in all files list?		     */
readme_ext='.README'			/* Files with name ending with this string   */
					/* are not displayed in filelistings, but    */
					/* they are shown with VIEW,INFO and SHOW    */
    subj='REQUEST'			/* This needs to be the subject, otherwise   */
					/* message isn't processed (case is ignored) */
    sdir='packed:Fileserver/'		/* Root directory, must end with / or :      */
   logon='Packed:Fileserver.logon'	/* This message is copied to the beginning of*/
  					/* a reply. For example inform about changes */
allfiles=''				/* This is used for ALLFILES command, if the */	
					/* string is not empty and the file exists   */
helpfile='Packed:Fileserver.help'       /* This file is reply to HELP command	     */
  seslog='packed:session.log'		/* Stores all requests and their results     */
seslogto=''				/* If this is not empty, session log will be */
					/* mailed there when the script ends 	     */
 logfile='packed:fileserver.log'	/* Session logs are stored here */
     log='yes'				/* If something else than yes, no logging    */
autosend='yes'				/* If yes, messages are sent by the script   */
  delmsg='yes'				/* Deletes messages after processing. If this*/
  					/* is something else than yes, messages are  */
					/* just marked as deleted                    */

replsubj='Fileserver reply from kajun@sci.fi'

type='c:type'		/* AmigaDOS command type */
list='c:list'		/* AmigaDOS command list */
copy='c:copy'
rename='c:rename'	/* Yet another command */
lha='c:lha'		/* This is used to display .lha and .lzh files */
unzip='c:unzip'		/* .zip */
unlzx='c:unlzx'		/* .lzx */




/*
** BE VERY CAREFUL WHEN YOU CHANGE SOMETHING BELOW!!!
*/

if upper(opt1)='INDEX' & allfiles~='' then do
	call CreateListing(sdir,all)
	address command copy 't:list.output' allfiles
	call delete('t:list.output')
	exit
	end

if pos(right(sdir,1),':/')=0 then sdir=sdir'/'

/* Open seslog, if needed */
log_it=0
listed_already=0
if upper(log)='YES' then log_it=open(4,seslog,'w')

address 'YAM'
'Hide'
'SetFolder' folder

'GetMailInfo Active'	/* Remember current message */
active=result
if rc>0 then active=0

'GetFolderInfo Max'	/* How many messages are there? */
n=result

/* Check all messages in folder */
do m=0 to n-1
	'SetMail' m
	/* proces if correct header */
	'GetMailInfo Subject'
	if upper(result)=upper(subj) then do
		'GetMailInfo From'
		s=result
		'GetMailInfo File'
		msgfile=result
		call Process(s,result)
		if upper(delmsg)='YES' then
			call delete(msgfile)
		else
			'MailDelete'
		end
	end

if upper(delmsg)='YES' then 'MailUpdate'

if log_it then do
	call close(4)				/* Close session log  */
	address command type '>>'logfile seslog	/* Copy the contents */
	end

if seslogto~='' & log_it then do
	'MailWrite'
	'WriteMailTo "'seslogto'"'
	'WriteSubject "Session log from fileserver"'
	'WriteLetter' seslog
	'WriteQueue'
	end

address command 'delete >nil:' seslog
if upper(autosend)='YES' then 'MailSendAll'

if listed_already then call delete('t:full.list')
'SetMail' active
'Show'
exit

CopyFile:
parse arg copyname
	if exists(copyname) then do
		call open(cp,copyname,'r')
		do while ~eof(cp)
			r=readln(cp)
			call writeln(2,r)
			end
		call close(cp)
		if log_it then call writeln(4,copyname 'copied to reply')
		end
	else
		if log_it then call writeln(4,'*** Failed to copy' copyname 'into reply ***')
return

Process:
parse arg sender,file
	if log_it then 
		call writeln(4,left('0a'x sender '--' date() '--' time()' ',71,'-'))
	call open(1,file,'r')	
	call open(2,'t:fileserver.reply','w')
	call CopyFile(logon)
	att=0
	dir=sdir
	help=0
	shrthlp=0
	'MailWrite'
	'WriteMailTo "'sender'"'
	'WriteSubject "'replsubj'"'

	/* Skip message headers */
	do until r='' | eof(1)
		r=strip(readln(1))
		end

	/* process body: until empty or quit*/
	do until upper(r)='QUIT' | eof(1)
		r=translate(strip(readln(1)),'/','\')
		call writeln(2,'>' r)
		cmd=upper(word(r,1))
		ok=0
		if cmd='ALLFILES' | cmd='LISTALL' then do
			call send_allfiles
			ok=1
			end
		if cmd='QUIT' then ok=1
		if cmd='GET' | cmd='SEND' then do
			call send_file(word(r,2))
			ok=1
			end
		if cmd='CD' then do
			call change_dir(word(r,2))
			ok=1
			end
		if cmd='VIEW' | cmd='SHOW' | cmd='INFO' then do
			call view_file(word(r,2))
			ok=1
			end
		if cmd='INDEX' | cmd='LIST' | cmd='DIR' then do
			call send_index(word(r,2))
			ok=1
			end
		if cmd='HELP' then do
			call send_help
			ok=1
			end
		if ~ok & r~='' then do
			call writeln(2,'Unknown command!')
			if log_it then
	                        call writeln(4,'*** Unknown command' r '***')   
			if ~shrthlp then do
				call writeln(2,'Try: ALLFILES, CD [path], DIR [path], GET <file>, HELP, INDEX [path],')
				call writeln(2,'     INFO <file>, LIST [path], LISTALL, SEND <file>, SHOW <file>')
				call writeln(2,'     or VIEW <file>')
				call writeln(2,'[path] is optional and <file> means a file in the current directory')
				shrthlp=1
				end
			end
		address 'YAM'
		end
	call close(1)
	call close(2)
	'WriteLetter t:fileserver.reply'
	'WriteQueue'
return

send_help:
	if help then do
		call writeln(2,'Once should be enough...')
		return
		end
	call CopyFile(helpfile)
	help=1
return

send_allfiles:
	if allfiles~='' & exists(allfiles) then do
		call CopyFile(allfiles)
		return
		end
	if ~listed_already then do
		call CreateListing(sdir,all)
		address command rename 't:list.output t:full.list'
		listed_already=1
		end
	call CopyFile('t:full.list')
	if log_it then call writeln(4,'List of all files sent')
return

send_index:
parse arg path
	if left(path,1)~='/' then
		foo=dir || path
	else
		foo=sdir || substr(path,2)
	if exists(foo) then do
		call CreateListing(foo,'')
		call CopyFile('t:list.output')
		if log_it then call writeln(4,'Index of' foo 'sent')
		call delete('t:list.output')	
		end
	else do
		call writeln(2,'Directory does not exist')
		if log_it then
                        call writeln(4,'*** Tried list' foo '***')   
		end	
return

change_dir:
parse arg newdir
	if newdir='' then do
		dir=sdir
		call writeln(2,'Directory changed back to root.')
		if log_it then
			call writeln(4,'CD to root')
		return
		end
	if pos(right(newdir,1),'/:')=0 then newdir=newdir'/'
	if left(newdir,1)='/' then
		foo=sdir || substr(newdir,2)
	else
		foo=dir || newdir
	if exists(foo) then do
		inf=statef(foo)
		if word(inf,1)='DIR' then do
			dir=foo
			call writeln(2,'Ok')
			if log_it then
				call writeln(4,'CD to' newdir 'ok')
			end
		else do
			call writeln(2,'It is not a directory!')
			if log_it then
				call writeln(4,'*** Tried to cd to a file' newdir '***')
			end
		end
	else do
		call writeln(2,'No such subdirectory!')
		if log_it then
			call writeln(4,'*** Tried to cd to a nonexistent' newdir '***')
		end
return


view_file:
parse arg filename
	foo=dir || filename
	if exists(foo) then do
		inf=statef(foo)
		parse var inf ft byt blo flgs days min ticks comment
		call writeln(2,'Filetype:'ft ' Size:'byt 'bytes  Last modified:'date('n',days,'i'))
		call writeln(2,strip(comment))
		if exists(foo || readme_ext) then call CopyFile(foo || readme_ext)
		if log_it then
			call writeln(4,'Displayed file info of' foo)
		ext=upper(right(filename,4))
		ar=0
		if ext='.LHA' | ext='.LZH' then do
			address command lha '>t:arc.list v' foo
			ar=1
			end
		if ext='.ZIP' then do
			address command unzip '>t:arc.list -v' foo
			ar=1
			end
		if ext='.LZX' then do
			address command unlzx '>t:arc.list v' foo
			ar=1
			end
		if ar=1 then do
			call CopyFile('t:arc.list')
			call delete('t:arc.list')
			if log_it then call writeln(4,'Displayed archive contents of' foo)
			end
		end
	else do
		call writeln(2,'No such file!')
		if log_it then
			call writeln(4,'*** Failed to display info of' foo '***')
		end
return

send_file:
parse arg fn
	if exists(dir || fn) then do
		ft='application/octet-stream'
		p=lastpos('.',fn)
		if p>0 then do
			ext=upper(substr(fn,p+1))
			if ext='GIF' then ft='image/gif'
			if ext='JPG' then ft='image/jpeg'
			if ext='WAV' then ft='audio/basic'
			if ext='TXT' then ft='text/plain'
			if ext='REXX' then ft='text/plain'
			if ext='MPG' then ft='video/mpeg'
			end
		'WriteAttach' dir || fn '"" MIME' ft
		att=att+1
		call writeln(2,att'. file included as an attachment.')
		if log_it then
			call writeln(4,'File' dir||fn 'sent')
		end /* if exists(dir ll fn) then */
	else do
		call writeln(2,'File not found!')
		if log_it then
			call writeln(4,'*** Could not send non-existent' dir || fn '***')
		end
return

CreateListing:
parse arg lstdir,opt

if pos(right(lstdir,1),'/:')=0 then lstdir=lstdir'/'
if opt='' then
	address command list '>t:lst.tmp1 lformat="%P%S %B %C"' lstdir
else
	address command list '>t:lst.tmp1 lformat="%P%S %B %C" all files' lstdir

call open(in,'t:lst.tmp1','r')
call open(out,'t:lst.tmp2','w')
longest=0

do while ~eof(in)
	r=readln(in)
	if length(r)>length(rootdit) then r=substr(r,length(lstdir)+1)
	parse var r name .
	po=lastpos('/',name)
	if longest<length(name)-po then longest=length(name)-po
	if upper(right(name,7))~=upper(readme_ext) then call writeln(out,r)
	end

call close(in)
call delete('t:lst.tmp1')
call close(out)

call open(in,'t:lst.tmp2','r')
call open(out,'t:list.output','w')

do while ~eof(in)
	r=readln(in)
	if r~='' then do
		parse var r name size comment
		comment=strip(comment)
		if size='empty' then size=0
		if comment='' then comment='(no description available)'
		if datatype(size)='NUM' then do
			if size<2000 then
				size=trunc(size/2+0.5)'K'
			else
				size=trunc(size/2048+0.05)'M'
			end
		sc=length(name)-length(compress(name,'/'))
		le=longest+sc*dirle/* Dir names should be at most dirle letters */
		line=left(name,le) right(size,4) comment
		if length(line)>maxline then do	
			po=lastpos(' ',line,maxline)
			rest=left('',le+6) || substr(line,po+1)
			line=left(line,po-1)
			do while strip(rest)~=''
				po=lastpos(' ',rest,maxline)
				if po<=le+6 then po=maxline
				if length(rest)>maxline then do
					line=line || '0a'x || left(rest,po-1)
					rest=left('',le+6) || substr(rest,po+1)
					end
				else do
					line=line || '0a'x || rest
					rest=''
					end
				end
			end
		call writeln(out,line)
		end
	end
call close(in)
call delete('t:lst.tmp2')
call close(out)
return
