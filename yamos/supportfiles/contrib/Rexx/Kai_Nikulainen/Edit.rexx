/*$VER:Edit.rexx     © Kai Nikulainen                                 email: kajun@sci.fi 1.0 (0-0-0)
Quickly edits a message in any folder.

** Just select a message and start this script.  You may want to change
** 'sys:tools/memacs' to some other editor and it might not support the
** start line like µemacs ('GOTO 12') */

options results

address 'YAM' 'GetMailInfo File'
address command 'sys:tools/memacs' result 'GOTO 12'
exit
