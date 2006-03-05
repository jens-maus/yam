/* $VER:ChangeConfig.rexx 1.0 - Kai Nikulainen (10-Apr-97)
Changes the YAM's configuration. 
**
** Because there is no ARexx command for loading a configuration file, the
** script needs to quit YAM and then start it again with another config.
**
** If you leave the opts.x variable empty YAM will check for new mail as soon
** as the new configuration is loaded.
**
** If you have any problems or suggestions, mail knikulai@utu.fi
*/

options results
call addlib('rexxsupport.library',0,-30,0)	/* Delay needs this */

/*
** Change the following variables to suit your system:
*/

configs=2		/* Number of different configurations */
file.1='YAM:.config'	/* Config file 1 */
 but.1='_Normal'	/* Requester button for config 1.  _ means keyboard shortcut */
 dir.1='YAM:'		/* Maildir for this configuration */
opts.1='NOCHECK'	/* Usable options here are NOCHECK, HIDE, DEBUG, POP3=xxxx */
			/* and SMTP=xxxx.  The variable can be left empty */

file.2='YAM:.testconfig'
 but.2='_Test'
 dir.2='YAM:'
opts.2='NOCHECK'

text='Change configuration to'

/*
** You shouldn't need to change anything below....
*/

buttons=''
do i=1 to configs
	buttons=buttons || but.i || '|'
	end
buttons=buttons || '_Cancel'

address 'YAM' 
'request "'text'" "'buttons'"'
if result=0 then exit
i=result

'Quit'
call delay(50)

do while pos(p,'YAM')	/* Just to make sure YAM isn't running anymore */
	call delay(50)	/* It is probably unnecessary */
	end

address command 'run >nil: YAM:YAM prefsfile='file.i 'maildir='dir.i opts.i
exit
