/* $VER: GEOhide.rexx 1.0 - 21-Apr-97 by Kai.Nikulainen@sci.fi
** Hides the first part of a multipart message from YAM 
*/ 
options results
   reply='yes'			/* if this yes, starts to reply to the processed message */
     tmp='t:letter.tmp'
HideWith='$'

address 'YAM'
'GetMailInfo file'
if rc>0 then do
	'Request "Select a message first!" "_Ok"'
	exit
	end
else
	file=result

if open(in,file,'r') & open(out,tmp,'w') then do
	BeenThereDoneThat=0
	do until r='' | eof(in)
		r=readln(in)
		call writeln(out,r)
		if word(r,1)='Content-Type:' & ~BeenThereDoneThat then do
			r=translate(r,' ','"')
			boundary=word(r,words(r))
			BeenThereDoneThat=1
			end /* if word */
		end /* do until */
	NothingHiddenYet=1
	do until eof(in) 
		r=readln(in)
		if pos(boundary,r)>0 & NothingHiddenYet then do
			NothingHiddenYet=0
			r=HideWith || r
			end
		if ~eof(in) then call writeln(out,r)
		end /* do until */
	call close(in)
	call close(out)
	address command 'c:copy' tmp file
	address command 'c:delete' tmp
	if upper(reply)='YES' then 'MailReply'
	end /* if open */
else do
	'Request "Can not copy letter!" "_Ok"'
	exit
	end

exit
