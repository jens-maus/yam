/* $VER:ShowSubjects.rexx  © Kai Nikulainen                            email: kajun@sci.fi 1.1 (1-Jun-97)
Adds the subject of the message as a filenote for easier recognition in Directory utilities.
*/

options results
call addlib('rexxsupport.library',0,-30,0)
rmsg='Messages in the folder*nnow have filenotes.'
rbut='_Ok'

address 'YAM'
'Hide'
'GetMailInfo Act'       /* Which message is currently selected */
act=result              /* Try to remember it */
'MailUpdate'            /* Let's make sure the index is up to date */
'GetFolderInfo Max'     /* How many messages are there? */
n=result

do m=0 to n-1           /* Do for all messages in folder: */
        'SetMail' m             /* Select a message */
        'GetMailInfo File'      /* Get the filename */
        file.m=result           /* Save the filename */
        info=statef(result)     /* Get all kinds of info about the file */
        parse var info type bytes blocks flags days min ticks comment
        'GetMailInfo Subject'   /* Guess what it does now? */
        subj=translate(result,"'",'"*')
        if subj='' then subj='(No subject)'
        comm.m=strip(left(left(strip(comment),2)||subj,79)) /* When you start stripping, do it properly :-) */
end /* do m */

/* Now we know what needs to be done, let's do it! */
do m=0 to n-1
        address command 'filenote' file.m '"'comm.m'"'
        if rc>0 then say 'filenote' file.m '"'comm.m'"'
end

'SetMail' act           /* Return to the message which was previously selected */
'Show'
'request "'rmsg'" "'rbut'"'
exit
