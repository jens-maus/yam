/* $VER LoadConfig.rexx v 1.0 by Kai Nikulainen
** Restarts YAM with new configuration. 
**
** Because there is no ARexx command for loading a configuration file, the
** script needs to quit YAM and then start it again with another config.
**
** Clone this script and you can have own menu item for each configuration.
**
** If you have any problems or suggestions, mail kajun@sci.fi
*/

options results
call addlib('rexxsupport.library',0,-30,0)	/* Delay needs this */

/*
** Change the following variables to suit your system:
*/

file='YAM:.config'	/* Load this config file */
 dir='YAM:'		/* Maildir for this configuration */
opts=''			/* Usable options here are NOCHECK, HIDE, DEBUG, POP3=xxxx */
			/* and SMTP=xxxx.  The variable can be left empty */

address 'YAM' 'Quit'
call delay(50)

do while pos(p,'YAM')	/* Just to make sure YAM isn't running anymore */
	call delay(50)	/* It is probably unnecessary */
	end

address command 'run >nil: YAM:YAM prefsfile='file 'maildir='dir opts
exit
