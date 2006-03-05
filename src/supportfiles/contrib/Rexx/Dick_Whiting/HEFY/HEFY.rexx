/******************************************************************************/
/*                                                                            */
/*                           HEFY.rexx                                        */
/*                      (Html Email For Yam)                                  */
/*                                                                            */
/*               for use with YAM 2.0 by Marcel Beck                          */
/*                                                                            */
/*                 Copyright ©1999 by Dick Whiting                            */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*  This one was specifically requested by a friend that has done             */
/*  translations for me on several of my programs. I _personally_ do NOT      */
/*  like receiving these, but he wanted it...                                 */
/*                                                                            */
/*  To use:                                                                   */
/*                                                                            */
/*     Place script in Yam:rexx and add to your URB buttons, or Yam menu.     */
/*                                                                            */
/*     READ comments within this script and edit as indicated to your taste   */
/*                                                                            */
/*     Create a signature file or files (HEFY.sig is an example)              */
/*     You need to know basic HTML to edit these properly.                    */
/*                                                                            */
/*     !! Remember to update sigfiles.0 and sigfiles.n in this script         */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*  On my 040 this script takes about 6 seconds for a "normal" 3K mail and    */
/*  about 13 seconds for a hundred lines of heavily styled mail.              */
/*  Be Patient when running it:)                                              */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/* Standard Disclaimer: I wrote it, it works for me, I don't guarantee        */
/* that it will do anything productive for anyone else, etc. etc. ;-)         */
/*                                                                            */
/* HOWEVER, if you DO find a use for it: I homeschool my kids and they        */
/* would love a postcard from where EVER you live.                            */
/*                                                                            */
/* Instant GEOGRAPHY lesson;)                                                 */
/*                                                                            */
/*                                                                            */
/* POSTCARDS:    Dick Whiting                                                 */
/*               28590 S. Beavercreek Rd.                                     */
/*               Mulino, Oregon 97042                                         */
/*               USA                                                          */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*               Address Bug Reports or Comments to:                          */
/*                Dick Whiting <dwhiting@europa.com>                          */
/*                            06 Oct 1999                                     */
/*                                                                            */
/*   Other Scripts & Programs available at http://www.europa.com/~dwhiting/   */
/*                                                                            */
/******************************************************************************/
/*
$VER: 1.6 Copyright ©1999 by Dick Whiting
$AUTHOR: Dick Whiting
$DESCRIPTION: Creates an HTML attachment for the selected OUTGOING mail text
$REQUIRES: Works only with Yam2.x by Marcel Beck
*/

/**************************************************************************/
/*         !! READ COMMENTS AND EDIT THE FOLLOWING VALUES !!              */ 
/**************************************************************************/

/**************************************************************************/
/*        The name of YOUR OutGoing folder if localized or changed        */
/**************************************************************************/

OutName='Outgoing'                          /* example: German 'Ausgang'  */

/**************************************************************************/
/*             Select which lines appear as HTML email header             */
/*  Set =1 to include it =0 to not                                        */
/*  e.g. ShowReplyto=1 will have it display the Replyto: header           */
/**************************************************************************/

ShowFrom=1                                  /* From:    header line       */
ShowReplyto=1                               /* Replyto: header line       */
ShowTo=1                                    /* To:      header line       */
ShowDate=1                                  /* Date:    header line       */
ShowSubject=1                               /* Subject: header line       */

/**************************************************************************/
/*    Information from YAM that may be turned on as extra header lines    */
/**************************************************************************/
/*  This information is only accurate BEFORE the script is run and before */
/*  the mail is moved to the Sent (or other) folder.                      */
/**************************************************************************/

ShowFilename=0                              /* Filename known to YAM      */
ShowStatus=0                                /* YAM status of mail         */
ShowFlags=0                                 /* YAM flags for mail         */
ShowSize=0                                  /* Size of original mail      */

/**************************************************************************/
/*                        User defined header lines                       */
/*  These will appear after the From: To: ... Flags: Size: ones           */
/*  Set UserHead.0 to the number defined (UserHead.0=0 for NO User heads) */
/*  Set UserHead.n to the title of the field                              */
/*  Set UserText.n to the information to display for UserHead.n           */
/**************************************************************************/
/**************************************************************************/
/*       Note: These lines are processed the same as a YAM text line      */
/**************************************************************************/

UserHead.0=1
UserHead.1='X-Mailer:'
UserHead.2=''
UserHead.3=''
UserHead.4=''
UserHead.5=''

/**************************************************************************/
/*       Note: These lines are processed the same as a YAM text line      */
/**************************************************************************/

UserText.1='YAM 2.0 [040] AmigaOS E-Mail Client (c) 1995-1999 by Marcel Beck  http://www.yam.ch'
UserText.2=''
UserText.3=''
UserText.4=''
UserText.5=''

/**************************************************************************/
/*               You can set these HTML COLOR control values              */
/**************************************************************************/
/*  You can use either the named color or the RGB hex values              */
/*  Examples: link='BLUE' or link='0000FF'                                */
/**************************************************************************/
 
link='BLUE'                                 /* Color for LINKS            */
alink='RED'                                 /* While link is being pressed*/
vlink='DARK BLUE'                           /* Color for visited links    */

color.0='BLACK'                             /* Normal text color          */
color.1='WHITE'                             /* One level of quoting       */
color.2='YELLOW'                            /* Two levels of quoting      */
color.3='AQUA'                              /* Color for YAM style        */

bgcolor='GRAY'                              /* Background color           */
background=''                               /* URL for a bg picture       */

/**************************************************************************/
/*             URL of a _small_ (100x100) picture of yourself             */
/*                  Leave as mypic='' to NOT include one                  */
/**************************************************************************/

mypic=''                                    /* URL for your picture       */
mypictext='Picture of me'                   /* ALT text for picture       */

/**************************************************************************/
/*      Signature files available for signing HTML mail                   */
/*      Set sigfile.0 to the number of signatures available               */
/* You'll be prompted for which one to use if there's more than one       */
/**************************************************************************/

sigfile.0=1                                  /* number of signatures      */
sigfile.1='yam:rexx/HEFY.sig'                /* signature 1 for HTML mail */
sigfile.2=''                                 /* signature 2 for HTML mail */
sigfile.3=''                                 /* signature 3 for HTML mail */
sigfile.4=''                                 /* signature 4 for HTML mail */
sigfile.5=''                                 /* signature 5 for HTML mail */

/**************************************************************************/
/*        Special character to indicate setting/unsetting <PRE> tag       */
/*    The YAM line must have ONLY this character on it to be recognized   */
/*    Place one just prior to the block and one immediately following it  */
/**************************************************************************/
/*    Use this to include ASCII art, diagrams, etc. that require fixed    */
/*              fonts and use spaces for positioning of text              */
/**************************************************************************/

PreChar='-'                                 /* toggles <PRE> on/off       */

/**************************************************************************/
/*  Special character to indicate including block without ANY processing  */
/*    The YAM line must have ONLY this character on it to be recognized   */
/*    Place one just prior to the block and one immediately following it  */
/**************************************************************************/
/*    Use this to include HTML commands that you wish included ASIS       */
/*    bypassing Yam styling, character encoding, etc.                     */
/**************************************************************************/

HtmlChar='+'                             /* toggles HTML including on/off */

/**************************************************************************/
/*                           HTML header values                           */
/*         Set meta.0 to the number of META statements you define         */
/**************************************************************************/

meta.0=2
meta.1='   <META content=text/html;charset=iso-8859-1 http-equiv=Content-Type>'
meta.2='   <META content="HEFY.rexx by Dick Whiting for YAM 2.0 ©Marcel Beck" name=GENERATOR>'

/**************************************************************************/
/*                   Temp files used by this script                       */
/**************************************************************************/

temphtml='T:email.html'                     /* temporary HTML file        */
tempmail='T:temp.mail'                      /* temporary MAIL file        */

/**************************************************************************/
/*                    !! NO NEED TO EDIT BEYOND HERE !!                   */
/**************************************************************************/

Address YAM
options results

signal off syntax     /* Change this to 'signal ON syntax' for debug file */

'GETFOLDERINFO NAME'
fname=result

'MAILINF STEM' inf.                         /* get basic header stuff     */

if upper(fname)~=upper(OutName) | rc>0 then do
   'request "Select a mail in your 'OutName' folder" "OK"'
   exit 
end

'MAILREAD QUIET'
'READSAVE PART 1 FILENAME 'tempmail         /* Get text of current email  */
rs=rc

'READCLOSE'

if rs>0 then exit                           /* User Canceled              */

'APPBUSY "Doing HTML attachment...Please be patient..."'

'MAILEDIT'

if open('OUT',temphtml,'W') then do
   Call Setup      
   Call HTMLheader
   Call MailHeader
   Call CopyToSig
   Call HTMLtail
end
else do
   'request "Unable to open' temphtml 'for output" "OK"'
   Call Cleanup
end

foo=close('OUT')

'WRITEATTACH' temphtml                      /* Attach it                  */
'WRITEQUEUE'                                /* Put it in the Queue        */

Call Cleanup

exit


/**************************************************************************/
/*                      Build the HTML header stuff                       */
/**************************************************************************/

HTMLheader:

   foo=writeln('OUT',htmldoc)
   foo=writeln('OUT','<HTML>')
   foo=writeln('OUT','<HEAD>')
   do i=1 to meta.0
      foo=writeln('OUT',meta.i)
   end
   foo=writeln('OUT','</HEAD>')
   
   bodystr='<BODY'
   if bgcolor~='' then bodystr=bodystr||' BGCOLOR="'||bgcolor||'"'
   if background~='' then bodystr=bodystr||' BACKGROUND="'||background||'"'

   if color.0~='' then bodystr=bodystr||' TEXT="'||color.0||'"'
   if link~='' then bodystr=bodystr||' LINK="'||link||'"'
   if alink~='' then bodystr=bodystr||' ALINK="'||alink||'"'
   if vlink~='' then bodystr=bodystr||' VLINK="'||vlink||'"'

   bodystr=bodystr||'>'
   foo=writeln('OUT',bodystr)

return

/**************************************************************************/
/*            Build and write header (From: To: Date: Subject:)           */
/**************************************************************************/
MailHeader:

   encstr=inf.from
   Call EncodeLatin
   encfrom=encstr

   if ShowReplyto then do
      encstr=inf.replyto
      Call EncodeLatin
      encreplyto=encstr
   end

   if ShowTo then do
      encstr=inf.to
      Call EncodeLatin
      encto=encstr
   end

   if ShowDate then do
      dateout=inf.date
      parse var dateout mm '-' dd '-' yy hhmmss
      select
         when yy < 78  then yy=2000 + yy
         when yy < 100 then yy=1900 + yy
         otherwise nop
      end
      dayofwk=left(date('W',yy||mm||dd,'S'),3)
      dateout=dayofwk||', '||dd||'-'||word(months,mm)||'-'||yy||' at '||hhmmss
   end

   encstr=inf.subject
   Call EncodeLatin
   encsubj=encstr

   if ShowFilename then do
      encstr=inf.filename
      Call EncodeLatin
      encfilename=encstr
   end

   if ShowStatus then do
      encstr=inf.status
      Call EncodeLatin
      encstatus=encstr
   end

   if ShowFlags then do
      encstr=inf.flags
      Call EncodeLatin
      encflags=encstr
   end

   if ShowSize then do
      encstr=inf.size
      Call EncodeLatin
      encsize=encstr
   end

   foo=writeln('OUT','<TABLE BORDER="0" CELLPADDING="0" CELLSPACING="0">')
   
   if ShowFrom then do
      foo=writeln('OUT','<TR><TD><B>From:</B></TD>')
      foo=writeln('OUT','<TD>')
      if mypic~='' then do
         foo=writeln('OUT','<IMG SRC="'||mypic||'" ALT="'||mypictext||'"><BR>')
      end
      foo=writeln('OUT','<A HREF="mailto:'||encfrom||'?subject='||encsubj||'">'||encfrom||'</A>')
      if mypic~='' then do
         foo=writeln('OUT','<BR>&nbsp;<BR>')
      end
      foo=writeln('OUT','</TD></TR>')
   end

   if ShowReplyto then do
      foo=writeln('OUT','<TR><TD><B>Reply-To:&nbsp;</B></TD>')
      foo=writeln('OUT','<TD>')
      foo=writeln('OUT','<A HREF="mailto:'||encreplyto||'?subject='||encsubj||'">'||encreplyto||'</A>')
      foo=writeln('OUT','</TD></TR>')
   end

   if ShowTo then do
      foo=writeln('OUT','<TR><TD><B>To:&nbsp;</B></TD>')
      foo=writeln('OUT','<TD>'||encto||'</TD></TR>')
   end

   if ShowDate then do
      foo=writeln('OUT','<TR><TD><B>Date:&nbsp;</B></TD>')
      foo=writeln('OUT','<TD>'||dateout||'</TD></TR>')
   end

   if ShowSubject then do
      foo=writeln('OUT','<TR><TD><B>Subject:&nbsp;</B></TD>')
      foo=writeln('OUT','<TD>'||encsubj||'</TD></TR>')
   end

   if ShowFilename then do
      foo=writeln('OUT','<TR><TD><B>Filename:&nbsp;</B></TD>')
      foo=writeln('OUT','<TD>'||encfilename||'</TD></TR>')
   end

   if ShowStatus then do
      foo=writeln('OUT','<TR><TD><B>Status:&nbsp;</B></TD>')
      foo=writeln('OUT','<TD>'||encstatus||'</TD></TR>')
   end

   if ShowFlags then do
      foo=writeln('OUT','<TR><TD><B>Flags:&nbsp;</B></TD>')
      foo=writeln('OUT','<TD>'||encflags||'</TD></TR>')
   end

   if ShowSize then do
      foo=writeln('OUT','<TR><TD><B>Size:&nbsp;</B></TD>')
      foo=writeln('OUT','<TD>'||encsize||'</TD></TR>')
   end

   do ucnt=1 to UserHead.0
      encstr=UserHead.ucnt
      Call EncodeLatin
      foo=writeln('OUT','<TR><TD><B>'||encstr||'&nbsp;</B></TD>')
      linein=UserText.ucnt
      if QuickTest() then do
         lineout=''
         Call Styling
         Call Buildline
         foo=writeln('OUT','<TD>'||lineout||'</TD></TR>')
      end
      else do
         encstr=linein
         Call EncodeLatin
         foo=writeln('OUT','<TD>'||encstr||'</TD></TR>')
      end
   end

   foo=writeln('OUT','</TABLE>')
   foo=writeln('OUT','<BR>')

return

/**************************************************************************/
/*     Read Temp Mail file until a '-- ' signature indicator is found     */
/**************************************************************************/
CopyToSig:

   if open('IN',tempmail,'R') then do
      do until eof('IN')
         linein=readln('IN')
         if linein~="-- " then do
            if linein=PreChar then do 
               if PreOn then foo=writeln('OUT','</PRE><BR>')
               else foo=writeln('OUT','<PRE>')
               PreOn=~PreOn                 /* toggle <PRE> on/off        */
               iterate
            end
            if PreOn then do
               foo=writeln('OUT',linein)
               iterate
            end
            if linein=HtmlChar then do 
               HtmlOn=~HtmlOn               /* toggle HTML inclusion      */
               iterate
            end
            if HtmlOn then do
               foo=writeln('OUT',linein)
               iterate
            end
            Call Quoting
            if QuickTest() then do
               Call Styling
               Call BuildLine
               foo=writeln('OUT',lineout||'<BR>')
            end
            else do
               encstr=linein
               Call EncodeLatin
               foo=writeln('OUT',lineout||encstr||'<BR>')
               encstr=''
            end
         end
         else leave
      end
      if PreOn then foo=writeln('OUT','</PRE>') /* turnoff before sigfile */
   end
   else do
      'request "Unable to open' tempmail 'for output" "OK"'
      Call Cleanup
   end

   foo=close('IN') 

return

/**************************************************************************/
/*                  Copy the HTML sig file and close tags                 */
/**************************************************************************/

HTMLtail:

   if sigfile.0 > 0 then do
      if sigfile.0 > 1 then do
         rbody='Select a Signature File\n\n'
         rgad=''
         do i=1 to sigfile.0
            rbody=rbody||i||'  '||sigfile.i||'\n'
            rgad=rgad||i||'|'
         end
         rgad=rgad||'None'

         'request "'rbody'" "'rgad'"'
         signum=result
         usesig=sigfile.signum
      end
      else do
         signum=1
         usesig=sigfile.1
      end

      if signum>0 then do

         if open('IN',usesig,'R') then do
            do until eof('IN')
               linein=readln('IN')
               foo=writeln('OUT',linein)
            end
         end
         else do
            'request "Unable to open' usesig 'for input" "OK"'
         end
         foo=close('IN') 
      end
   end

   foo=writeln('OUT',"</BODY></HTML>")

return

/**************************************************************************/
/*                       Handle YAM quoting rules                         */
/**************************************************************************/

Quoting:

    q1pos=0
    q2pos=0
    lineout=''
    
    if linein='' then do
       if cnum>0 then do
          lineout='</FONT>'
          cnum=0
       end
       return
    end

    tstquote=substr(linein,1,9)
    q1pos=pos('>',tstquote)

    if q1pos>0 then do
       if cnum>1 then lineout='</FONT>'
       if cnum~=1 then lineout=lineout||'<FONT COLOR="'color.1'">'
       cnum=1
       encstr=substr(tstquote,1,q1pos-1)
       Call EncodeLatin
       lineout=lineout||encstr||'&gt;'
       tstquote=delstr(tstquote,1,q1pos)
       if left(tstquote,1,1)=' ' then do
          lineout=lineout||'&nbsp;'
       end
       q2pos=pos('>',tstquote)
       if q2pos>0 then do
          if cnum~=2 then do
             lineout=lineout||'</FONT><FONT COLOR="'color.2'">'
             cnum=2
          end
          encstr=substr(tstquote,1,q2pos-1)
          Call EncodeLatin
          lineout=lineout||encstr||'&gt;'
          tstquote=delstr(tstquote,1,q2pos)
          if left(tstquote,1,1)=' ' then do
             lineout=lineout||'&nbsp;'
          end
       end
       linein=substr(linein,q1pos+q2pos+1)
    end
    else do
       if cnum=2 then do
          lineout='</FONT><FONT COLOR="'color.1'">'
          cnum=1
       end
    end

return

/**************************************************************************/
/*           Quick Test to see if any styling or urls to handle           */
/**************************************************************************/

QuickTest: Procedure expose linein upperlinein

   upperlinein=upper(linein)               /* do it here for later tests */

   if verify(linein,'*/_#','M')>0 then return 1
   if pos("MAILTO:",upperlinein)>0 then return 1

return 0

/**************************************************************************/
/*                               TextStyling                              */
/**************************************************************************/
/*                                RULES                                   */
/*     1) Start if first character of line or prev char NOT alphanum      */
/*     2) Ending if start-to-end includes an alphanum and not in a URL    */
/**************************************************************************/

Styling:
   
   linecode.=' '                            /* styling url information    */
   curpos=1                                 /* track location in linein   */

   Call Nextpos                             /* Locate next style or URL   */
   linelen=length(linein)

   do while (npos>0 & curpos<linelen)
      select
         when linecode.npos~=' ' then nop    /* already handled this char */
         when verify(curchar,'*/_#','M')>0 then do
            if GoodPrev(prevchar) then do
               if ~CharInUrl(npos) then do
                  epos=FindMatch(npos)
                  if epos>0 then do
                     Call FillCodes
                  end
               end
            end
         end
         when curchar=':' then do
            epos=FindUrlEnd(npos)
            Call FillCodes
            npos=epos                       /* bump to end of URL         */
         end
         when curchar='M' then do
            epos=FindMailEnd(npos)
            if epos>0 then do
               Call FillCodes
               npos=epos                    /* bump to end of mail addr   */
            end
            else npos=npos+6                /* bump to end of mailto:     */
         end
         otherwise nop
      end
      curpos=npos+1                         /* keep looking from here     */
      if curpos<linelen then Call Nextpos   /* Locate next style or URL   */
      
   end

return

/**************************************************************************/
/*            Find the position of next style or url to handle            */
/**************************************************************************/

Nextpos:

   spos=verify(substr(linein,curpos),'*/_#','M')
   if spos>0 then spos=spos+curpos-1

   hpos=pos('://',linein,curpos)
   mpos=pos('MAILTO:',upperlinein,curpos)

   select
      when spos=0 & hpos=0 & mpos=0 then npos=0

      when spos>0 & hpos=0 & mpos=0 then npos=spos
      when spos=0 & hpos>0 & mpos=0 then npos=hpos
      when spos=0 & hpos=0 & mpos>0 then npos=mpos

      when spos>0 & hpos>0 & mpos=0 then npos=min(spos,hpos)   
      when spos>0 & hpos=0 & mpos>0 then npos=min(spos,mpos)   
      when spos=0 & hpos>0 & mpos>0 then npos=min(hpos,mpos)   

      when spos>0 & hpos>0 & mpos>0 then npos=min(spos,hpos,mpos)   
      otherwise npos=0
   end

   if npos>0 then curchar=upper(substr(linein,npos,1))
             else curchar=''

   if npos>1 then prevchar=substr(linein,npos-1,1)
   else prevchar=' '                        /* previous character tests   */

   if curchar=':' then do
      npos=lastpos(' ',linein,npos)         /* find blank before '://'    */
      if npos=0 then npos=1                 /* line starts with URL       */
      else npos=npos+1                      /* actual start of URL        */
   end

return

/**************************************************************************/
/*                  Test if a valid start for YAM styling                 */
/**************************************************************************/

GoodPrev: 

   if c2d(prevchar)>126 | pos(prevchar,alphanums)>0 then return 0

return 1

/**************************************************************************/
/*                     Is style character within a URL                    */
/**************************************************************************/

CharInUrl: Procedure expose linein

   parse arg cpos

   do j=1 to words(linein)                 /* start of word with char.   */
      wpos=wordindex(linein,j)
      if wpos=cpos then leave j
      if wpos>cpos then do
         j=j-1
         leave j
      end
   end

   select                          /* Ends inside a URL??        */
      when pos("://",word(linein,j))>0 then return 1
      when pos("MAILTO:",word(upperlinein,j))>0 then return 1
      otherwise nop
   end

return 0

/**************************************************************************/
/*                     Find matching styling character                    */
/**************************************************************************/

FindMatch: Procedure expose linein curchar alphanums

   parse arg spos 

   epos=0
   do j=spos+1 to length(linein)
      if substr(linein,j,1)=curchar then do
         if ~CharInUrl(j) then do
            if verify(substr(linein,spos+1,j-spos),alphanums,'M')>0 then do
               epos=j
               leave
            end
         end
      end
   end

return epos

/**************************************************************************/
/*                         Find the end of the URL                        */
/**************************************************************************/
/**************************************************************************/
/*         !!! Should change to locate first illegal URL character        */
/*               !!! Step backwards if last character is '.'              */
/**************************************************************************/

FindUrlEnd: Procedure expose linein htmlchars

   parse arg spos

   epos=verify(substr(linein,spos),htmlchars)-1

   if epos<0 then epos=length(linein)
             else epos=spos+epos-1

   if substr(linein,epos,1)='.' then epos=epos-1 /* sentence ending       */

return epos

/**************************************************************************/
/*                      Find end of a Mailto: string                      */
/**************************************************************************/

FindMailEnd: Procedure expose linein

   parse arg spos

   epos=pos('@',linein,spos)                /* find @ following mailto:   */
   if epos> 0 then do                       /* found one                  */
      epos=pos(' ',linein,epos+1)-1         /* find following blank       */
      if epos<0 then epos=length(linein)    /* assume rest of line        */
   end

return epos

/**************************************************************************/
/*                  Fill in stem array for styling & URLs                 */
/**************************************************************************/

FillCodes: 

   spos=npos

   select
      when curchar='*' then do
         linecode.spos = startbold 
         linecode.epos = endbold 
      end
      when curchar='/' then do
         linecode.spos = startitalic
         linecode.epos = enditalic
      end
      when curchar='_' then do
         linecode.spos = startunder
         linecode.epos = endunder
      end
      when curchar='#' then do
         linecode.spos = startcolor
         linecode.epos = endcolor
      end
      otherwise do
         linecode.spos = curchar
         linecode.spos.1 = epos-spos+1
         do j=spos+1 to epos
            linecode.j=0
         end
      end
   end

return 

/**************************************************************************/
/*             Use linecode stem array to build output string             */
/**************************************************************************/

BuildLine:

   encstr=''
   codestr=''

   do i=1 to length(linein)
      codestr=codestr||linecode.i
   end

   spos=1
   codepos=verify(substr(codestr,spos),' ')
   if codepos>0 then codepos=codepos+spos-1

   do while codepos>0
      lcode=substr(codestr,codepos,1)
      if codepos-spos>0 then do
         encstr=substr(linein,spos,codepos-spos)
      end

      if length(encstr)>0 then do
         Call EncodeLatin
         lineout=lineout||encstr
         encstr=''
      end

      if lcode<=styles.0 then do
         lineout=lineout||styles.lcode
         spos=codepos+1
      end
      else do
         lineout=lineout||'<A HREF="'
         encstr=substr(linein,codepos,linecode.codepos.1)
         Call EncodeLatin
         lineout=lineout||encstr||'">'||encstr||'</A>'
         encstr=''
         spos=codepos + linecode.codepos.1
      end
            
      codepos=verify(substr(codestr,spos),' ')
      if codepos>0 then codepos=codepos+spos-1

   end

   if spos<=length(codestr) then do
      encstr=encstr||substr(linein,spos)
   end

   if length(encstr)>0 then do
      Call EncodeLatin
      lineout=lineout||encstr
      encstr=''
   end
   

return

/**************************************************************************/
/*                 Encode Latin-1 characters to html form                 */
/**************************************************************************/

EncodeLatin:

enccopy=encstr
encstr=''

litpos=verify(enccopy,valid_chars)

do while litpos>0 & length(enccopy)>0
   if litpos>1 then do 
      encstr=encstr||substr(enccopy,1,litpos-1)
   end
   encdec=c2d(substr(enccopy,litpos,1))
   encstr=encstr||enc.encdec
   enccopy=delstr(enccopy,1,litpos)
   litpos=verify(enccopy,valid_chars)   
end
encstr=encstr||enccopy

return


/**************************************************************************/
/*          Setup Latin-1 to html table and other variables               */
/**************************************************************************/

Setup:       

PreOn=0                                     /* Flag for <PRE> </PRE>      */
HtmlOn=0                                    /* Flag for HTML inclusion    */
cnum=0                                      /* Current Quoting color      */

htmldoc='<!DOCTYPE HTML PUBLIC "-//W3C//DTD W3 HTML//EN">'

months='Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec'

startbold   = 1
endbold     = 2
startitalic = 3
enditalic   = 4
startunder  = 5
endunder    = 6
startcolor  = 7
endcolor    = 8

styles.0=8
styles.1='<B>'
styles.2='</B>'
styles.3='<I>'
styles.4='</I>'
styles.5='<U>'
styles.6='</U>'
styles.7='<FONT COLOR="'||color.3||'">'
styles.8='</FONT>'

alphanums="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"

htmlchars=alphanums||"#$%&'()*+,-./:;=?@_~"

valid_chars=" !#$%'()*+,-./0123456789:;=?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~"

enc.=''
enc.34 ='&quot;'
enc.38 ='&amp;'           
enc.60 ='&lt;'
enc.62 ='&gt;'
enc.161='&iexcl;'
enc.162='&cent;'
enc.163='&pound;'
enc.164='&curren;'
enc.165='&yen;'
enc.166='&brvbar;'
enc.167='&sect;'
enc.168='&uml;'
enc.169='&copy;'
enc.170='&ordf;'
enc.171='&laquo;'
enc.172='&not;'
enc.174='&reg;'
enc.175='&macr;'
enc.176='&deg;'
enc.177='&plusmn;'
enc.178='&sup2;'
enc.179='&sup3;'
enc.180='&acute;'
enc.181='&micro;'
enc.182='&para;'
enc.183='&middot;'
enc.184='&cedil;'
enc.185='&sup1;'
enc.186='&ordm;'
enc.187='&raquo;'
enc.188='&frac14;'
enc.189='&frac12;'
enc.190='&frac34;'
enc.191='&iquest;'
enc.192='&Agrave;'
enc.193='&Aacute;'
enc.194='&Acirc;'
enc.195='&Atilde;'
enc.196='&Auml;'
enc.197='&Aring;'
enc.198='&AElig;'
enc.199='&Ccedil;'
enc.200='&Egrave;'
enc.201='&Eacute;'
enc.202='&Ecirc;'
enc.203='&Euml;'
enc.204='&Igrave;'
enc.205='&Iacute;'
enc.206='&Icirc;'
enc.207='&Iuml;'
enc.208='&ETH;'
enc.209='&Ntilde;'
enc.210='&Ograve;'
enc.211='&Oacute;'
enc.212='&Ocirc;'
enc.213='&Otilde;'
enc.214='&Ouml;'
enc.215='&times;'
enc.216='&Oslash;'
enc.217='&Ugrave;'
enc.218='&Uacute;'
enc.219='&Ucirc;'
enc.220='&Uuml;'
enc.221='&Yacute;'
enc.222='&THORN;'
enc.223='&szlig;'
enc.224='&agrave;'
enc.225='&aacute;'
enc.226='&acirc;'
enc.227='&atilde;'
enc.228='&auml;'
enc.229='&aring;'
enc.230='&aelig;'
enc.231='&ccedil;'
enc.232='&egrave;'
enc.233='&eacute;'
enc.234='&ecirc;'
enc.235='&euml;'
enc.236='&igrave;'
enc.237='&iacute;'
enc.238='&icirc;'
enc.239='&iuml;'
enc.240='&eth;'
enc.241='&ntilde;'
enc.242='&ograve;'
enc.243='&oacute;'
enc.244='&ocirc;'
enc.245='&otilde;'
enc.246='&ouml;'
enc.247='&0divide;'
enc.248='&oslash;'
enc.249='&ugrave;'
enc.250='&uacute;'
enc.251='&ucirc;'
enc.252='&uuml;'
enc.253='&yacute;'
enc.254='&thorn;'
enc.255='&yuml;'

return

/**************************************************************************/
/*                       Cleanup on Error Condition                       */
/**************************************************************************/

Cleanup:

   foo=close('IN')
   foo=close('OUT')

   Address Command 'Delete ' tempmail 'Quiet'
   Address Command 'Delete ' temphtml 'Quiet'

   'APPNOBUSY'

exit

/**************************************************************************/
/*      Error debugging routine -- Set 'signal ON syntax' to enable       */
/**************************************************************************/

Error:
Syntax:

   errcode=rc
   if open('BUG','t:HEFY.debug','W') then do
      foo=writeln('BUG','Error in line: '||sigl)
      foo=writeln('BUG','Sourceline: '||sourceline(sigl))
      foo=writeln('BUG','Error:   '||errcode)
      foo=writeln('BUG','Err Msg: '||errortext(errcode))
      foo=writeln('BUG','Linein : '||linein)
      foo=writeln('BUG','Codestr: '||codestr)
      foo=writeln('BUG','Codepos: '||codepos)
      foo=writeln('BUG','Lineout: '||lineout)
      foo=writeln('BUG','Encstr: '||encstr)
      foo=writeln('BUG','i spos epos: '||i spos epos)
   end
   foo=close('BUG')

   foo=close('IN')
   foo=close('OUT')

   'APPNOBUSY'

   'REQUEST "An unexpected error has occurred\nPlease email file t:HEFY.debug\nto dwhiting@europa.com" "OK"'
exit
