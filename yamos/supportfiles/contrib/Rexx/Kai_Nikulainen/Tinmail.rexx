/* TINmail.rexx v1.0 by knikulai@utu.fi
**
** Send mail from tin with YAM
** 
** Installation:
**  -type 'protect TINmail.rexx +s' in shell
**  -set the environment variable TIN_MAILER to this script
**  -set in tinrc: default_mailer_format=%M %F %T "%S"
**
** If YAM was not running when the script is executed, it will be quitted.
*/
parse arg file addr '"'subj'"'

hideyam='yes'	/* If yes, hides YAM which makes the script faster and it's harder */
		/* to accidentally mess things up */
showyam='no'	/* If yes, shows (=unhides) YAM when the script is over */
temp='t:tinmail.temp'

call addlib('rexxsupport.library',0,-30,0)

if ~show('P','YAM') then do
	say 'Starting YAM'
	address command 'run >nil: yam:yam nocheck'
	call delay(50)
	address command 'waitforport YAM'
	quityam=1
	end
else
	quityam=0

if show('P','YAM') then do
	call StripHeaders(file)
	address 'YAM'
	if upper(hideyam)='YES' then 'Hide'
	'MailWrite'
	'WriteMailTo "'addr'"'
	'WriteSubject "'subj'"'
	'WriteLetter' temp
	'WriteQueue'
	if upper(showyam)='YES' then 'Show'
	end
else
	address command 'RequestChoice "TINmail error" "Could not run YAM!" "Ok"'
call delete(temp)
exit

StripHeaders:
parse arg fn
if open(1,fn,'r') & open(2,temp,'w') then do
	body=0
	do while ~eof(1)
		r=readln(1)
		if r='' then body=1
		if (~eof(1) & body) | word(r,1)='Newsgroups:' then call writeln(2,r)
		end /* while */
	call close(1)
	call close(2)
	call delete(fn)
	end
else do
	address command 'RequestChoice "TINmail error" "Could not remove headers" "Ok"'
	temp=fn
	end
return	
