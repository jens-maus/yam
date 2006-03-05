/* $VER: YAMmail.rexx 1.2 by knikulai@utu.fi
** Check http://www.sci.fi/~kajun for more YAM scripts
** 
** IBrowse: Network Preferences/Email & Telnet/Mailto:
**          Type: External
**          Command: rx YAM:Rexx/YAMmail.rexx "%h" %s
*/

Options Results
parse arg '"'email'"' subject
if pos('No_subject',subject)>0 then subject='Mail from your web page'

if ~show('P','YAM') then do           /* YAM is not running yet, so let's start it */
  address command 'run <>nil: yam:yam nocheck'
  i=0
  do until show('P','YAM') | i=3     /* Let's not wait more than 30 seconds */
    address command 'sys:rexxc/WaitForPort YAM'
    i=i+1
  end /* do */
  if show('P','YAM')=0 then exit
end

address 'YAM'  			/* The following commands are sent to YAM */
'Show'				/* Uniconify */
'MailWrite'    			/* Press 'Write'-button */
'WriteMailTo "'||email||'"'     /* Set address */
'WriteSubject "'||subject||'"' 	/* Set subject */
exit
