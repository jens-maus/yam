/*
** $VER: ExchangeMailbox.rexx 1.1 (31.10.96)
**
** Written by Marcin Orlowski <carlos@inet.com.pl>
** 
** This script launches Miami and YAM (if not launched),
** establishes Internet connection (if not established)
** then gets all new mails and sends all "Outgoing".
**
** History:
**
** v1.0  - initial release
** v1.1  - disconects Miami only when we had to
**         establish connection, otherwise just
**         quits.
*/

/*
** Some variables and pathes
*/

MIAMI      = "INET:Miami/Miami"
MIAMI_FIND = 'WaitForPort MIAMI.1'
YAM        = "YAM:YAM"
YAM_FIND   = 'WaitForPort YAM'

/*
** Whenever we need to launch Miami or YAM we note
** this to quit only those app we launched
*/

YAM_CALL   = 0
MIAMI_CALL = 0

/*
** Let's see if apps are running. If not, we call them
** Unfortunately command WaitForPort makes few tries
** before it return so it may take a while if apps are
** not running. You may use other tool instead.
*/

ADDRESS COMMAND MIAMI_FIND
IF RC = 5 THEN
  DO
   MIAMI_CALL = 1
   ADDRESS COMMAND "C:Run <>NIL: " || MIAMI
  END

ADDRESS COMMAND YAM_FIND
IF RC = 5 THEN
  DO
   YAM_CALL = 1
   ADDRESS COMMAND "C:Run <>NIL: " || YAM
  END

/*
** Small trick to wait until YAM finish its set up
** You may remove these lines if it takes longer
** for Miami to establish the connection than
** YAM to set up (don't forget: Amiga multitasks!)
** This depends on Miami setup (PULSE dialing,
** long number) and numer of mails in YAM folders
** especially if it needs to rebuild the index file
*/

ADDRESS YAM
setfolder 0

/*
** Go online (if we are currently not)
*/

ADDRESS MIAMI.1

MAIMIONLINE = 0
IsOnline
IF RC = 0 THEN
   DO
    OnLine
    MIAMIONLINE = 1
   END

/*
** Let's exchange mailbox
*/

ADDRESS YAM
mailcheck
setfolder 'outgoing'
mailsend all

/*
** Let's clean up
*/

IF MIAMIONLINE THEN
  DO
   ADDRESS MIAMI.1
   OffLine
  END

IF MIAMI_CALL THEN
  DO
    ADDRESS MIAMI.1
    Quit
  END

IF YAM_CALL THEN
  DO
    ADDRESS YAM
    Quit
  END
