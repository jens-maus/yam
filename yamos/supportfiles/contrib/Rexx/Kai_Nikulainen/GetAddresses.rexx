/* $VER:GetAdresses.rexx  © Kai Nikulainen                               email: kajun@sci.fi 1.2 (24-Sep-97)
Gets addresses from mails in a folder
*/

options results
addlib('rexxreqtools.library',0,-30)
NL='0a'x    
     scrn=''            /* Name of YAM's screen where the requester appears */
    title='GetAddresses.rexx by Kai.Nikulainen@sci.fi'
  sea_txt='Which messages should be searched?'
  sea_but='_All of them|_Selected message|_Cancel'
  cho_txt='Do you want to'
  cho_but='_Save Address|_Write Mail|_Cancel'
  sel_txt='Select address to save in addressbook'
  sel_but='_Next Page|_Cancel'
   yn_but='_Yes, I certainly do!|_No, I do not!'
error_txt='Could not write to file '
addressbook='YAM:.addressbook'
ignore='Received: Return-Path:' /* If the first word of the line is one of these, no */
                                /* addresses will be searched from that line */

ac=0    /* address count */

address 'YAM'
'GetMailInfo Active'
act=result
'GetFolderInfo Max'
last=result-1
first=0

if datatype(act)='NUM' then do
        sea=rtezrequest(sea_txt,sea_but,title,'rtez_defaultresponse=2 rt_pubscrname='scrn)
        if sea=0 then exit
        if sea=2 then do
                last=act
                first=act
                end     
        end
        
do m=first to last
   'SetMail' m
   'GetMailInfo File'
   call ScanFile(result)
end /*do m=0 to n-1*/
'SetMail' act

first=1
do until sel=0
        body=ac 'addresses were found:'NL
        buttons=''
        i=first
        lab=0
        do while lab<9 & i<=ac
           body=body || i-first+1 || ') ' || name.i email.i || NL
           buttons=buttons'_'|| i-first+1 || '|'
           lab=i-first+1        /* last address button */
           i=i+1
           end /* do while */
        buttons=buttons || sel_but
        
        sel=rtezrequest(body,buttons,title,'rtez_defaultresponse=0 rt_pubscrname='scrn)
        if sel>0 then do
            if sel>lab then do
                first=first+lab
                if first>ac then first=1
                end /* if sel>lab then do */
            else do /* sel<=lab */
                j=first+sel-1
                cho=rtezrequest(cho_txt,cho_but,title,'rtez_defaultresponse=1 rt_pubscrname='scrn)
                if cho=0 then exit
                if cho=2 then do
                        'MailWrite'
                        'WriteMailTo' email.j
                        exit
                        end
                call AddToAddresbook(j)
            end /* else do */               
        end /* if sel>0 then do */
end /* do until sel=0 */
exit

AddToAddresbook:
   parse arg ind
   na=rtgetstring(name.ind' ',"Edit  name",title,'_Ok|_Forget It!','',s1)
   if s1=0 then return
   ad=rtgetstring(email.ind,'Edit address for' na,title,,,se) 
   if ad='' then return
   al=rtgetstring(alias.ind' ','Edit alias for' na 'at' ad,title,,'',s2)
   if s2=0 then return
   de=rtgetstring('Friend','Enter description for' na 'at' ad,title,,,s3)
   if s3=0 then return
   body='Do you want to add the following'NL'lines to' addressbook '?' NL NL
   body=body '@USER' al NL ad NL na NL de NL '@ENDUSER' NL NL
   body=body 'Remember to reload the addressbook after changes!'
   if rtezrequest(body,yn_but,title,'rtez_defaultresponse=0') then do
       if open(out,addressbook,'A') then do
          call writeln(out,'@USER' al)
          call writeln(out,ad)
          call writeln(out,na)
          call writeln(out,de)
          call writeln(out,'@ENDUSER')
          call close(out)
          end
       else
          call rtezrequest(error_txt addressbook,,title,'rtez_defaultresponse=0')
       end
return

ScanFile:
  parse arg fname
  if open(1,fname,'R') then do
    do until eof(1)
        rivi=translate(readln(1),' ',',<>()"~$;'||'09'x,' ')
        if pos('@',rivi)>0 & pos(word(rivi,1),ignore)=0 then do
            fn=''
            ln=''
            ad=''
            do w=1 to words(rivi)
               fn=ln
               ln=ad
               ad=word(rivi,w)
               if right(ad,1)=':' then ad=''
               if pos('@',ad)>0 & ad~='@' then call AddIt(ad fn ln)
               end /* do w=1 */
        end /* if pos */
    end /* do until eof(1) */
    call close(1)
  end /* if open(1, */
return

AddIt:
  parse arg addy etu suku
  foo=upper(addy)
  k=1
  do while k<=ac
     if foo=upper(email.k) then return  /* The address is already there, go home */
     k=k+1
     end
  ac=ac+1                       /* Prepare to add new entry... */
  email.ac=strip(addy)          /* address */
  name.ac=strip(etu suku)       /* name */
  alias.ac=strip(etu)           /* alias. First name should be reasonable default */
return
