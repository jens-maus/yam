/*$VER:FixQuotes.rexx   © Kai Nikulainen                                email: kajun@sci.fi 1.0 (15-Mar-97)
Removes unsupported font characters from messages
*/

options results
tempfile='t:FixQuotes' || random(1,1000,time(s)) || time(s)

address 'YAM'
'GetMailInfo File'

if rc>0 then do /* You screwed up! */
        'Request "You need to select a message first!" "_Abort script"'
        exit /* quit script */
        end  /* not the script, the do after then... */

filename=result
if open(in,filename,'r') then do
        if open(out,tempfile,'w') then do      /* try to open a temp file */
                do while ~eof(in)             /* convert whole message */
                        r=readln(in)          /* read a line */
                        r=change(r,'=92',"'") /* I've no idea what this */
                        r=change(r,'92'x,"'") /* char is called... :-)  */
                        r=change(r,'=91',"`") /* the same char */
                        r=change(r,'91'x,"`") /* again */
                        r=change(r,'=93','"') /* begin quote */
                        r=change(r,'93'x,'"') /* so is this */
                        r=change(r,'=94','"') /* end quote */
                        r=change(r,'94'x,'"') /* same here */
                        call writeln(out,r)
                        end
                call close(out)
                end
        else do
                'Request "Can not write temp file!" "_Abort script"'    /* Bad luck! */
                exit
                end
        call close(in)
        address command 'copy >nil:' tempfile filename
        address command 'delete >nil:' tempfile
        'Request "All quotes converted." "Ok"'
        end
else
        'Request "Can not open message!" "_Abort script"'/* This shouldn't ever happen */
exit    /* That's all folks! */

change:
parse arg s,o,n                 /* Args: string, old chars, new chars */
do while pos(o,s)>0             /* do while there are old chars left */
        p=pos(o,s)              /* find the position */
        b=left(s,p-1)           /* store the beginning of line */
        e=substr(s,p+length(o)) /* and the end of line */
        s=b || n || e           /* create new and improved string */
        end
return s                        /* Return the result */
