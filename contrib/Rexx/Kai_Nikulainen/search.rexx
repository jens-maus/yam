/* $VER:Search.rexx  © Kai Nikulainen                                email: kajun@sci.fi 1.1 (5-Feb-97)
Searches messages for given word

  Send comments, suggestions, bug reports and e-money to kajun@sci.fi
  Remember to check http://www.sci.fi/~kajun for more scripts and new
  versions

*/
options results
call addlib('rexxreqtools.library',0,-30)

     title='Search.rexx by Kai.Nikulainen@utu.fi © 1996'
search_txt='Enter search word:'
 start_txt='Where should I start the search?'
 start_but='_First message|_Here|_Cancel'
 where_txt='From which messages you want to search?'
 where_but='_All|_Read|_Unread|_Not Replied|Cancel'
 found_txt='Word was found in this message'
 found_but='_Continue search|_End search'
    header='unchecked messages with word "'
   sel_but='_Scroll|_Done'
      tags='rt_pubscrname=YAMSCREEN'  /* Change here the name of the screen YAM runs */
maxmsg=9                /* Number of messages in one requester */
NL='0a'x                /* newline */

sana=rtgetstring('',search_txt,title,,tags)
if sana='' then exit
u_sana=upper(sana)
sel=rtezrequest(where_txt,where_but,title,'rtez_defaultresponse=1' tags)
if sel=0 then exit
types='ONUR'
if sel=2 then types='OR'
if sel=3 then types='UN'
if sel=4 then types='ONU'

address 'YAM'
firstmsg=0

sel=rtezrequest(start_txt,start_but,title,'rtez_defaultresponse=1' tags)
if sel=0 then exit
if sel=2 then do
        'GetMailInfo Active'
        firstmsg=result
        end

'GetFolderInfo Max'
n=result
mc=0
do m=firstmsg to n-1
   'SetMail' m
   'GetMailInfo Status'
   if pos(result,types)>0 then do  
      'GetMailInfo File'
      call open(in,result,r)
      found=0
      do until eof(in) | found>0
         rivi=readln(in)
         found=pos(u_sana,upper(rivi))
      end /* do until */
      call close(in)
      if found>0 then do /* Add this message to the list and display a requester*/
         mc=mc+1
         msgnum.mc=m
         'GetMailInfo Subject'
         msgsub.mc=result
         sel=rtezrequest(found_txt,found_but,title,'rtez_defaultresponse=1' tags)
         if sel=0 then exit
         end /* if found>0 */
   end /* if pos(result,types)>0 then do */
end /* do m=0 to n-1 */

curtop=1
do until mc=0
   if mc<=maxmsg then do
        curtop=1
        curbot=mc
        num=mc
        end
   else do
        curbot=curtop+maxmsg-1   
        num=maxmsg
        end
   body=mc header || sana || '"' || NL
   buttons=''
   do i=curtop to curbot
      j=i-curtop+1
      buttons=buttons ||'_' || j || '|'
      ind=1+i//mc
      body=body || j ') ' msgsub.ind || NL
      butnum.j=msgnum.ind
      butind.j=ind
   end /* do i=curtop to curbot */
   buttons=buttons || sel_but
   sel=rtezrequest(body,buttons,title,'rtez_defaultresponse=0' tags)
   if sel=0 then exit
   if sel=num+1 then do
        curtop=curtop+maxmsg
        if curtop>mc then curtop=curtop-mc
        end
   else do /* A message was selected */
say 'butind.sel=' butind.sel 'mc=' mc
      'SetMail' butnum.sel
      do i=butind.sel to mc-1
         k=i+1
         msgnum.i=msgnum.k
         msgsub.i=msgsub.k
         end
      mc=mc-1
   end /* else */
end /* do until mc=0 */
