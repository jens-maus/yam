/* Send mail from ALynx using YAM's editor 
   1. Make sure to protect this script with +s
   2. Edit lynx.cfg and make this script the MAILCOMMAND:
*/

Options Results
parse arg file 

Queue='yes'	/* If yes, the message is written to the queue automatically*/

if show('P','YAM')=0 then do           /* YAM is not running yet, so let's start it */
  address command 'run <>nil: yam:yam nocheck'
  i=0
  do until show('P','YAM')>0 | i=3     /* Let's not wait more than 30 seconds */
    address command 'sys:rexxc/WaitForPort YAM'
    i=i+1
  end /* do */
  if show('P','YAM')=0 then do         /* Something went wrong, let's quit */
    address command 'RequestChoice <>nil: "Error!" "Can not start YAM!" ":-(" pubscreen='||screen
    exit
  end
end

call open(1,file,'R')
call open(2,'t:LynxYAM','W')
do until eof(1)
  rivi=readln(1)
  /* The lines starting with following words are stripped from the mail*/
  if pos(word(rivi,1),'Subject: From: To: X-Personal_name: X-Mailer: X-URL:')>0 then do
    if word(rivi,1)='Subject:' then subject=substr(rivi,10)
    if word(rivi,1)='To:' then email=word(rivi,2)
    end
  else
    writeln(2,rivi)   
end
call close(1)
call close(2)
/* Now we know YAM is up and running */

address 'YAM'  			/* The following commands are sent to YAM */
'Show'				/* Uniconify */
'MailWrite'    			/* Press 'Write'-button */
'WriteMailTo "'||email||'"'     /* Set address */
'WriteSubject "'||subject||'"'  /* Set subject */
'WriteLetter t:LynxYAM'
if upper(Queue)='YES' then 'WriteQueue'
exit
