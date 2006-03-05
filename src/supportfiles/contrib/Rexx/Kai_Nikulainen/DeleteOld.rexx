/*$VER:DeleteOld.rexx  © Kai Nikulainen                              email: kajun@sci.fi 1.0 (22-Nov-97)
Marks all messages older than set limit as deleted.

This script will mark all messages older than limit days as deleted.  It doesn't
automatically remove them, unless variable auto_rem is set to 'YES'.  The date
of a message is read from the file, not from message headers.

Send suggestions and bug reports to kajun@sci.fi

Check http://www.sci.fi/~kajun for other scripts!
*/

options results
limit=30                /* All messages older than limit days, will be deleted */
ask_conf='YES'  /* If set to YES, a requester is displayed first */
auto_rem='NO'   /* If this variable is set to YES, messages are not just marked
                   as deleted, they are really removed from the disk.  There is
                   no way you can undelete them from YAM, so be careful! */
call addlib('rexxsupport.library', 0, -30, 0)
today=date(I)

address 'YAM'
if ask_conf=='YES' then do
   foo=date(N,today-limit,I)
   'Request "Are you sure you want to delete messages*nreceived before P[2]'||foo||'P[1]?" "_Yes|_No"'
   if result=0 then exit
end
'GetFolderInfo Max'
n=result

do msg=0 to n-1
    'SetMail' msg
    'GetMailInfo File'
    fname=result
    fi=statef(fname)
    parse var fi ft bytes blocks flags days .
    if limit<today-days then do
        if auto_rem=='YES' then
            call delete(fname)
        else
            'MailDelete'
    end /* if limit<today-days then */
end /*do msg=0 to n-1*/
if auto_rem=='YES' then address 'YAM' 'MailUpdate'
exit
