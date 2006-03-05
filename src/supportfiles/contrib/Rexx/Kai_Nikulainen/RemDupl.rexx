/*$VER:RemDupl      © Kai Nikulainen                                email: kajun@sci.fi 1.2 (27-Jun-97)
Finds and removes duplicate messages

** This version checks the message text instead of the headers!
**
** NOTE: You *MUST* set Configuration/Folder 'Delete messages on exit'
**       on or the script might delete wrong messages!  That also lets
**       you to undelete messages if something goes wrong.
*/
options results
call addlib('rexxsupport.library',0,-30,0)
limit=700       /* If the difference in file sizes is less than limit, the message */
                /* contents are checked */

address 'YAM'
'Hide'
'MailUpdate'
'GetFolderInfo Max'
num=result

do m=0 to num-1
        'SetMail' m
        'GetMailInfo File'
        file.m=result
        info=statef(result)
        parse var info type bytes .
        size.m=bytes
        msg.m=m
end /* do m */
size.num=9999999

call Sort(0,num-1)

dc=0
do m=0 to num-2
        i=m+1
        'SetMail' msg.m
        NotDeleted=1
        do while size.i-size.m<limit & NotDeleted & i<num
                if are_same(i,m) then do
                        'MailDelete'
                        NotDeleted=0
                        end
                i=i+1
                end
        end
'Show'
exit

are_same: procedure expose file. 
parse arg i,j
if open(1,file.i,'r') & open(2,file.j,'r') then do
        line1=SkipHeaders(1)
        line2=SkipHeaders(2)
        res=(line1=line2)
        do while res & ~eof(1) & ~eof(2)
                res=(readln(1)=readln(2))
                end
        end
else
        res=0
call close(1)
call close(2)
return res

SkipHeaders: procedure
parse arg f
        do until r='' | eof(f)
                r=readln(f)
                end
        do until r~='' | eof(f)
                r=readln(f)
                end
return r

Sort: procedure expose size. file. msg.
parse arg p,r
if p<r then do
        q=Partition(p,r)
        Call Sort(p,q)
        Call Sort(q+1,r)
        end
return

Partition: procedure expose size. file. msg.
parse arg p,r
x=size.p
i=p-1
j=r+1
do while 1
        do until size.j<=x
                j=j-1
                end
        do until size.i>=x
                i=i+1
                end
        if i<j then do
                t=size.j
                size.j=size.i
                size.i=t
                t=file.j
                file.j=file.i
                file.i=t
                t=msg.j
                msg.j=msg.i
                msg.i=t
                end
        else
                return j
        end

