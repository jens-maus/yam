/* $VER: AddXheader.rexx  © Kai Nikulainen                           email: kajun@sci.fi 1.1 (31-Oct-97)
Adds X-Headers from a given file to all messages in the outgoing folder. This version doesn't add same headers many times.  Messages edited by John Pullicino.
*/            

options results

headers='YAM:xheaders' /* File to read headers from. Empty lines are ignored */
			  /* Header format is NOT checked in any way, it's your */
			  /* responsibility */
tempfile='t:tmp.msg'

address 'YAM' 	  /* Our favourite program.. */
txt='"Shall I add the lines inP[5]'headers'P[1]*nto all the messages in this folder?"'
but='"_Yes|_No"'
address YAM
request txt but
if result=0 then exit

if ~open(hdr,headers,'r') then do     /* Something went wrong. Is the path correct? */
	'Request "Cannot findP[5]' headers'P[1] !*nP[2]~~~~~~~~~~~~~~~~~~~~~ P[1]*ncheck the name and path*n   of the headers file" "_Ok"'
	exit
	end

                       /* Read the header file:*/
lines=0
do while ~eof(hdr)
	li=strip(readln(hdr))
	if li~='' then do	/* Forget empty lines */
		lines=lines+1
		header.lines=li
		end
	end
call close(hdr)

'SetFolder 1'
'GetFolderInfo Max'
n=result
if n=0 then do
	'Request "The Outgoing folder is empty!" "_Ok"'
	exit
	end

do m=0 to n-1
	'SetMail' m		/* Change message */
	'GetMailInfo File'	/* Get it's filename */
	file=result
	address command 'c:copy' file tempfile	/* Copy the message */
	call open(in,tempfile,'r')		       /* Open copy for reading */
	call open(out,file,'w')			/* Open message for rewrite */

	body=0 				/* This means we are still in headers */
	add_here=0				/* Add headers when this is 1*/
	do while ~eof(in)
		li=readln(in)			/* Read a line from copy */
		if add_here & word(li,1)~=word(header.1,1) & ~body then do
			body=1			/* Make sure we wont do this again */
			do i=1 to lines	/* Write all headers */
				call writeln(out,header.i)
				end
			end /* if */
		call writeln(out,li)		/* Write the line */
		                                 /* Change the following line */
                                               /* to the header after which */
		                                 /* the new lines are added. */
		if word(li,1)='Date:' & ~body then 
			add_here=1
		else
			add_here=0
		end /* do while */
	call close(in)				/* Close copy */
	call close(out)				/* Close new message */
end
address command 'c:delete >nil:' tempfile	/* Delete the copy of the last msg */

exit
