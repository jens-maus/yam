/******************************************************************************/
/*                                                                            */
/*                            YTReFormat                                      */
/*                 Copyright ©1997 by Dick Whiting                            */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*    This version requires MuiRexx 3.0 use Yamtools 1.7 with MuiRexx 2.2     */
/*----------------------------------------------------------------------------*/
/* This script requires YAMTOOLS for it to work. If you don't have YAMTOOLS   */
/* you can get it on Aminet in directory comm/mail. This script allows you    */
/* to rewrap a mail file. I FINALLY got tired of doing it the hard way.       */
/* I would have liked to make it automatic, but just couldn't see how.        */
/* The current version will make all the quoting the same or remove it.       */
/* If you have any ideas for additional reformatting, contact me.             */
/*----------------------------------------------------------------------------*/
/* To use this select a mail and adjust the outer and inner quoting when      */
/* prompted. The outer quoting should be the character(s) or possibly none    */
/* that prefix the piece of lines that got wrapped badly. The inner quoting   */
/* should be the remainder (or none) necessary to make the lines uniform.     */
/* Sorry, if that is unclear. Play with it using the UNDO/REDO to figure      */
/* out how to make it work.                                                   */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Standard Disclaimer: I wrote it, it works for me, I don't guarantee        */
/* that it will do anything productive for anyone else, etc. etc. ;-)         */
/*                                                                            */
/*HOWEVER, if you DO find a use for it: I homeschool my kids and they         */
/*would love a postcard from where EVER you live.                             */
/*                                                                            */
/*Instant GEOGRAPHY lesson;)                                                  */
/*                                                                            */
/*                                                                            */
/*POSTCARDS:    Dick Whiting                                                  */
/*              28590 S. Beavercreek Rd.                                      */
/*              Mulino, Oregon 97042                                          */
/*              USA                                                           */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*               Address Bug Reports or Comments to:                          */
/*                Dick Whiting <dwhiting@europa.com>                          */
/*                           15 June 1997                                     */
/*                                                                            */
/******************************************************************************/
/*
$VER: 2.0 Copyright ©1997 by Dick Whiting
$AUTHOR: Dick Whiting
$DESCRIPTION: Re-Format a mail file - needs YAMTOOLS
*/

options results
options failat 21

wrapparm=''
parse arg mainparm subparm

/**************************************************************************/
/*                         Initialize Variables                           */
/**************************************************************************/
Call MUIvars                                /* go define vars for MUI use */
Call YTvars                                 /* various values used in YT  */
Call Helpvars                               /* pointers into HELP guide   */
Call Localize                               /* vars for localizing strings*/
Call Builtvars                              /* built using previous values*/

Address YAMTOOLS

Call CheckYam

Select
   when mainparm='' then do
      Call CheckDup
      Call GetYamInfo
      Call ReadMail
   end
   when mainparm='DONE' then do
      Call SaveFile   
      Call Quit
   end
   when mainparm='SAVE' then do
      Call SaveFile   
   end
   when mainparm='REWRAP' | mainparm='UNQUOTE' then do
      Call AskUser
   end
   when mainparm='DOREWRAP' | mainparm='DOUNQUOTE' then do
      Call ValidateUser
      Call WrapBlock
   end
   when mainparm='STYLE' then do
      Call Stylelines
   end
   when mainparm='STYLEIT' then do
      Call StyleIt
   end
   when mainparm='NEWHEAD' then do
      Call NewHead 
   end
   when mainparm='HEADIT' then do
      Call HeadIt
   end
   when mainparm='UNMIME' then do
      Call GetYamInfo
      method ID YTRFH MUIM_List_Clear
      method ID YTRFT MUIM_List_Clear
      method ID YTRFM MUIM_List_Clear
      Call UnMime
      Call ReadMail
   end
   when mainparm='UNDO' then do
      Call Undo
   end
   when mainparm='REDO' then do
      Call Redo
   end
   when mainparm='QUIT' then do
      Call Quit
   end
   otherwise do
      errmsg=_text._badparm
      Call ErrorMsg
   end
end 

exit

/**************************************************************************/
/*                        Read the mail file into array                   */
/**************************************************************************/
ReadMail:     

	header.=' '

   i=0
   header=TRUE
   quoted=FALSE

   if open('IN',mfile,'R') then do
      Call BuildWindow
      infotext=_text._procmail
      infobuttons=''
      showbusy=TRUE
      Call InfoWindow

      list ID YTRFT ATTRS MUIA_List_Quiet TRUE 
      list ID YTRFM ATTRS MUIA_List_Quiet TRUE 
      do until eof('IN')
         linein=readln('IN')
         linein=translate(linein,' ',tab) 
         if header then do
            list ID YTRFH INSERT POS MUIV_List_Insert_Bottom STRING '='linein
            if linein='' then do 
					header=FALSE
					do j=1 to 4
	               list ID YTRFT INSERT POS MUIV_List_Insert_Bottom,
							  STRING '='header.j
					end
				end
				select
	            when upper(word(linein,1))='FROM:' then header.1=linein
	            when upper(word(linein,1))='TO:'   then header.2=linein
	            when upper(word(linein,1))='DATE:' then header.3=linein
	            when upper(word(linein,1))='SUBJECT:' then header.4=linein
	            when upper(word(linein,1))='CONTENT-TRANSFER-ENCODING:' then do
                  encoding=word(linein,2)
                  if upper(encoding)='QUOTED-PRINTABLE' then quoted=TRUE
               end
					otherwise nop
				end
         end
         else do
            if ~eof('IN') then do
               linein=right(i,5,'0')||',='||linein
               list ID YTRFM INSERT POS MUIV_List_Insert_Bottom, 
                    ATTRS MUIA_List_Format '"WEIGHT=0 MAXWIDTH=0 MINWIDTH=0,"',
                    STRING linein
               i=i+1
            end
         end
      end
      foo=close('IN')
      list ID YTRFT ATTRS MUIA_List_Quiet FALSE
      list ID YTRFM ATTRS MUIA_List_Quiet FALSE
      window ID YTINF close
   end
   else do
      errmsg=_text._badmail
      Call ErrorMsg
      exit     
   end
   
   if quoted then do
      errmsg=_text._prtquoted
      Call ErrorMsg
      exit
   end
   
Return

/**************************************************************************/
/*                        Save the mail file back                         */
/**************************************************************************/
SaveFile:     

   infotext=_text._savemail    
   infobuttons=''
   showbusy=TRUE
   Call InfoWindow

   text ID YTRFF
   mfile=result

   fstate=statef(mfile)                           /* file information     */
   fbytes=subword(fstate,2,1)                     /* length of file       */
   fdate=subword(fstate,5,1)                      /* internal date        */
   fmins=subword(fstate,6,1)                      /* minutes since midnite*/
   fticks=subword(fstate,7,1)                     /* ticks past minutes   */
   fcomm=subword(fstate,8)                        /* old comment          */

   fdate=date('E',fdate,'I')                      /* get date in ddmmyy   */
   fdate=translate(fdate,'-','/')                 /* convert / to -       */
   hh=fmins%60                                    /* get hours            */
   hh=right(hh,2,'0')                             /* force to 2 digits    */
   mm=fmins//60                                   /* get minutes          */
   mm=right(mm,2,'0')                             /* force to 2 digits    */
   ss=fticks%50                                   /* seconds              */
   ss=right(ss,2,'0')                             /* force it             */
   ftime=hh||':'||mm||':'||ss                     /* timestamp rebuilt    */

   if open('OUT',mfile,'W') then do
      list ID YTRFH ATTRS MUIA_List_Entries       /* process header recs  */
      mcnt=result
      do i=0 to mcnt-1   
         list ID YTRFH POS i
         lineout=result
         foo=writeln('OUT',lineout)
      end
      list ID YTRFM ATTRS MUIA_List_Entries
      mcnt=result
      do i=0 to mcnt-1   
         list ID YTRFM POS i
         lineout=result
         lineout=substr(lineout,7)                /* strip number         */
         foo=writeln('OUT',lineout)
      end
      foo=close('OUT')
   end

   Address Command 'SetDate ' mfile fdate ftime

   window ID YTINF close
   
Return

/**************************************************************************/
/*                        Validate User Input                             */
/**************************************************************************/
ValidateUser:

   string ID RUSR1 
   OutQuote=result

   string ID RUSR2
   InnQuote=result

   string ID RUSR3
   WordWrap=result

   if ~datatype(WordWrap,'W') then do
      errmsg=_text._wwnotnum
      Call ErrorMsg
      exit     
   end

   wraplen=wordwrap-(length(OutQuote)+length(InnQuote))

   if WordWrap<wrapmin1 | WordWrap>wrapmax then do
      errmsg=_text._wwoutrange
      Call ErrorMsg
      exit     
   end

   if wraplen<wrapmin2 then do
      errmsg=_text._wwshort
      Call ErrorMsg
      exit     
   end

   window ID YTUSR close
   
Return

/**************************************************************************/
/*                        Do the actual wrapping                          */
/**************************************************************************/
WrapBlock:

   WWarray.=missing
   j=0

   startpos=0                            /* top of list for default       */

   list ID YTRFM ATTRS MUIA_List_Entries
   mcnt=result

   do i=0 to mcnt-1                      /* locate selected lines to wrap */
      list ID YTRFM 
      wstring=result
      if wstring='' then leave i
      j=substr(wstring,1,5)+0
      if startpos=0 then startpos=j      /* first line rewrapped          */
      WWarray.j=substr(wstring,7)        /* strip prefix stuff            */
   end

   if i=0 then do                        /* user didn't select any        */
      errmsg=_text._nolines          
      Call ErrorMsg
      exit     
   end

   if mainparm='DOREWRAP' then infotext=_text._dowrap
   else infotext=_text._dounquote
   infobuttons=''
   showbusy=TRUE
   Call InfoWindow

   foo=open('OLD','T:YTReForm.old','W')  /* files for undo/redo functions */
   foo=open('NEW','T:YTReForm.new','W')

   do i=0 to mcnt-1                      /* loop completely thru list     */
      list ID YTRFM POS i
      linein=result
      if WWarray.i=missing then do       /* not a rewrap line             */
         oldnum=left(linein,5)           /* the number portion            */
         oldval=substr(linein,7)         /* the mail line                 */
         lineout=oldnum||',='||oldval    /* rebuild with the equal sign   */
         foo=writeln('OLD',lineout)      /* save line to OLD file         */
         WWarray.i=substr(linein,7)      /* copy to new array             */
      end
      else do                            /* start of a block to rewrap    */
         tempstr=''                      /* init to null rewrap string    */
         do j=i to mcnt-1                /* loop thru the new array       */
            linein=WWarray.j             /* get highlighted string ?      */
            if linein=missing then leave j  /* block is complete          */
            WWarray.j=missing            /* clear the entry for now       */
            if OutQuote~='' then do
               if substr(linein,1,length(OutQuote))=OutQuote then do
                  linein=substr(linein,1+length(OutQuote))
               end
            end
            if InnQuote~='' then do
               if substr(linein,1,length(InnQuote))=InnQuote then do
                  linein=substr(linein,1+length(InnQuote))
               end
            end
            tempstr=tempstr||' '||linein /* unquoted text to rewrap       */
            list ID YTRFM POS j          /* keep copying to OLD           */
            linein=result
            oldnum=left(linein,5)        /* the number portion            */
            oldval=substr(linein,7)      /* the mail line                 */
            lineout=oldnum||',='||oldval /* rebuild with the equal sign   */
            foo=writeln('OLD',lineout)   /* save line to OLD file         */
         end         
         tempstr=substr(tempstr,2)       /* trim extra blank from start   */
         outlines=0                      /* none processed yet            */

         do while length(tempstr)>0      /* the actual rewrapping!!!      */
            blankpos=pos(' ',tempstr)    /* FIRST blank in remaining      */
            select
               when length(tempstr)<wraplen then do
                 newstr=tempstr
                 tempstr=''
               end
               when blankpos=0 then do
                 newstr=tempstr
                 tempstr=''
               end
               when blankpos>wraplen then do
                  newstr=substr(tempstr,1,blankpos)
                  tempstr=substr(tempstr,blankpos+1)
               end
               otherwise do
                  newstr=substr(tempstr,1,lastpos(' ',substr(tempstr,1,wraplen)))
                  tempstr=substr(tempstr,1+lastpos(' ',substr(tempstr,1,wraplen)))
               end
            end
            if mainparm='DOREWRAP' then do  /* if rewrapping add prefix   */
               newstr=OutQuote||InnQuote||newstr
            end
            k=i+outlines                 /* new pointer for array         */
            WWarray.k=newstr             /* store the wrapped line        */
            outlines=outlines+1
         end
         i=j-1                           /* decrement for loop control    */
      end
   end


/**************************************************************************/
/*  Clear the list, reload with rewrapped lines, copy to NEW file.        */
/**************************************************************************/

   list ID YTRFM ATTRS MUIA_List_Entries
   mcnt=result

   list ID YTRFM ATTRS MUIA_List_Visible
   lastpos=result

   jumppos=lastpos+startpos-3
   if jumppos>mcnt-1 then jumppos=mcnt-1

   list ID YTRFM ATTRS MUIA_List_Quiet TRUE 

   method ID YTRFM MUIM_List_Clear

   j=0
   do i=0 to mcnt-1
      if wwarray.i=missing then iterate  /* a null entry - skip it        */
      lineout=right(j,5,'0')||',='||wwarray.i
      foo=writeln('NEW',lineout)         /* write to the redo file        */
      list ID YTRFM INSERT POS MUIV_List_Insert_Bottom STRING lineout
      j=j+1                              /* pointer for list              */
   end

   button ID YTRFU ATTRS MUIA_Disabled FALSE   /* unghost UNDO button     */
   button ID YTRFR ATTRS MUIA_Disabled TRUE    /* ghost REDO button       */
   
   list ID YTRFM ATTRS MUIA_List_Quiet FALSE

   method ID YTRFM MUIM_List_Jump jumppos      /* go to first rewrapped   */

   foo=close('OLD')                      /* close undo file               */
   foo=close('NEW')                      /* close redo file               */

   window ID YTINF close

Return

/**************************************************************************/
/*  Prompt for new header line.                                           */
/**************************************************************************/
NewHead:    

   oldhead=translate(subparm,' ',tab) 
   headtype=' '||word(oldhead,1)
   oldhead=subword(oldhead,2)
   newhead=oldhead

   result=''
   text ID YTHD0
   if result='' then do
      Call HeadWin
   end
   else do
      text ID YTHD0 LABEL headtype
      string ID YTHD1 CONTENT oldhead
      string ID YTHD2 CONTENT newhead
   end
Return

/**************************************************************************/
/*  Process header changes.                                               */
/**************************************************************************/
HeadIt:

   if subparm='S' then do
      text ID YTHD0
      headtype=strip(result,'B')
      string ID YTHD2
      newhead=result
      list ID YTRFH ATTRS MUIA_List_Entries
      mcnt=result
      do i=0 to mcnt-1   
         list ID YTRFH POS i
         oldhead=result
         if substr(oldhead,1,length(headtype))=headtype then do
            newhead=headtype||' '||newhead
            list ID YTRFH POS i STRING newhead 
            list ID YTRFT ATTRS MUIA_List_Entries
            hcnt=result
            do j=0 to hcnt-1   
               list ID YTRFT POS j
               oldhead=result
               if substr(oldhead,1,length(headtype))=headtype then do
                  list ID YTRFT POS j STRING newhead 
                  leave j
               end
            end
            leave i
         end
      end
   end

   window ID YTHED close
   

Return
/**************************************************************************/
/*  Get first line and open window                                        */
/**************************************************************************/
StyleLines:

   wstring=''
   list ID YTRFM
   wstring=result
   if wstring='' then do
      errmsg=_text._noslines
      Call ErrorMsg
      exit
   end
   sline=substr(wstring,7)
   result=0
   list ID YTSTM ATTRS MUIA_List_Entries
   scnt=result
   if scnt=0 then do
      Call StyleWin
   end
   else do
      method ID YTSTM MUIM_List_Clear
   end
   string ID YTSTT CONTENT insert('=',wstring,6)
   string ID YTST1 CONTENT sline
   do i=1 to words(sline)
      slinex=right(i,2,'0')||',='||word(sline,i)
      list ID YTSTM INSERT POS MUIV_List_Insert_Bottom,
           ATTRS MUIA_List_Format '"WEIGHT=0 MAXWIDTH=0 MINWIDTH=0,"',  
           STRING slinex
   end
   maillist=sline
   Call AnsiStyle
   text ID YTST2 LABEL translate(maillist,' ',',')  

Return

/**************************************************************************/
/*  Perform commands from styling window                                  */
/**************************************************************************/
StyleIt:

   select
      when subparm='Q' then do
         list ID YTRFM POS 0
         list ID YTRFM
         window ID YTSTY close
      end
      when subparm='S' then do
         string ID YTSTT
         wstring=result
         wnum=substr(wstring,1,5)
         list ID YTRFM POS wnum STRING wstring
         list ID YTRFM
         wstring=result
         if wstring~='' then do
            method ID YTSTM MUIM_List_Clear
            sline=substr(wstring,7)
            string ID YTST1 CONTENT sline
            wstring=insert('=',wstring,6)
            string ID YTSTT CONTENT wstring
            do i=1 to words(sline)
               slinex=right(i,2,'0')||',='||word(sline,i)
               list ID YTSTM INSERT POS MUIV_List_Insert_Bottom,
                    ATTRS MUIA_List_Format '"WEIGHT=0 MAXWIDTH=0 MINWIDTH=0,"',
                    STRING slinex
            end
            maillist=sline
            Call AnsiStyle
            text ID YTST2 LABEL translate(maillist,' ',',') 
         end
      end
      when subparm='N' then do
         list ID YTRFM
         wstring=result
         if wstring~='' then do
            method ID YTSTM MUIM_List_Clear
            sline=substr(wstring,7)
            string ID YTST1 CONTENT sline
            wstring=insert('=',wstring,6)
            string ID YTSTT CONTENT wstring
            do i=1 to words(sline)
               slinex=right(i,2,'0')||',='||word(sline,i)
               list ID YTSTM INSERT POS MUIV_List_Insert_Bottom,
                    ATTRS MUIA_List_Format '"WEIGHT=0 MAXWIDTH=0 MINWIDTH=0,"',
                    STRING slinex
            end
            maillist=sline
            Call AnsiStyle
            text ID YTST2 LABEL translate(maillist,' ',',') 
         end
      end
      when subparm='E' then do
         string ID YTSTT
         wstring=result
         wnum=substr(wstring,1,5)
         string ID YTST1
         wstring=result
         if wstring~='' then do
            method ID YTSTM MUIM_List_Clear
            string ID YTSTT CONTENT wnum||',='||wstring
            do i=1 to words(wstring)
               slinex=right(i,2,'0')||',='||word(wstring,i)
               list ID YTSTM INSERT POS MUIV_List_Insert_Bottom,
                    ATTRS MUIA_List_Format '"WEIGHT=0 MAXWIDTH=0 MINWIDTH=0,"',
                    STRING slinex
            end
            maillist=wstring
            Call AnsiStyle
            text ID YTST2 LABEL translate(maillist,' ',',') 
         end
      end
      otherwise do
         list ID YTSTM ATTRS MUIA_List_Entries
         scnt=result
         do i=0 to scnt-1
            list ID YTSTM
            slinex=result
            if slinex='' then leave i
            snum=substr(slinex,1,2)+0
            string ID YTSTT
            wstring=result
            wnum=substr(wstring,1,5)
            slinex=substr(wstring,8)
            swordx=word(slinex,snum)
            spos=wordindex(slinex,snum)
            slinex=delstr(slinex,spos,length(swordx))
            select
               when subparm='B' then do 
                  swordx='*'||swordx||'*'      
                  stylechar='*'
               end
               when subparm='C' then do 
                  swordx='#'||swordx||'#'
                  stylechar='#'
               end
               when subparm='I' then do 
                  swordx='/'||swordx||'/'
                  stylechar='/'
               end
               when subparm='U' then do 
                  swordx='_'||swordx||'_'
                  stylechar='_'
               end
               otherwise do 
                  swordx=strip(swordx,'B','*')
                  swordx=strip(swordx,'B','#')
                  swordx=strip(swordx,'B','/')
                  swordx=strip(swordx,'B','_')
                  stylechar=''
               end
            end
            wstring=insert(swordx,slinex,spos-1)
            if snum>1 & stylechar~='' then do
               if substr(wstring,spos-2,1)=stylechar then do
                  wstring=delstr(wstring,spos,1)
                  wstring=delstr(wstring,spos-2,1)
               end
            end
            string ID YTSTT CONTENT wnum||',='||wstring
            string ID YTST1 CONTENT wstring
         end
         if i>0 then do
            method ID YTSTM MUIM_List_Clear
            do i=1 to words(wstring)
               slinex=right(i,2,'0')||',='||word(wstring,i)
               list ID YTSTM INSERT POS MUIV_List_Insert_Bottom,
                    ATTRS MUIA_List_Format '"WEIGHT=0 MAXWIDTH=0 MINWIDTH=0,"',
                    STRING slinex
            end
            maillist=wstring
            Call AnsiStyle
            text ID YTST2 LABEL translate(maillist,' ',',') 
         end
         else do
            errmsg=_text._noslines
            Call ErrorMsg
            exit
         end
      end
   end
Return

/**************************************************************************/
/*                        Convert styling to ANSI                         */
/*                                                                        */
/* Logic: Locate FIRST style character that fits rules.                   */
/*        Locate FIRST same  character after it.                          */
/*        If valid then replace those with ANSI codes.                    */
/*           Save that portion and loop for possible others.              */
/*        If NOT valid then loop looking for a new FIRST possibility.     */
/*                                                                        */
/* This appears to match the logic in Yam1.3.4 which probably will change */
/* in Yam2.x Right now it seems that the next matching style character    */
/* MUST be a valid ending one OR the starting one is invalid as a start.  */
/*                                                                        */
/**************************************************************************/
AnsiStyle:     
 
   qline=TRUE
   testword=word(maillist,1)
   if pos('>',testword)=0 then do 
      qline=FALSE
      qattr=qoff
   end

   mailtest=maillist
   mailcopy=''

   spos=verify(mailtest,stylelist,'MATCH')

   do while spos>0
      schar=substr(mailtest,spos,1)

      /* test if character BEFORE style character is a valid one */ 
      if spos>1 then do        
         tchar1=substr(mailtest,spos-1,1)
         if verify(goodpre1,tchar1,'MATCH')=0 & tchar1~=goodpre2 then do
            Call TrimIt
            iterate         
         end
      end

      /* Character BEFORE style character IS valid - test one after it */
      if spos<length(mailtest) then do
         tchar2=substr(mailtest,spos+1,1)
         if verify(badpost1,tchar2,'MATCH')>0 then do
            Call TrimIt
            iterate         
         end
      end
      else do   /* all done - style char was last char in string */
         mailcopy=mailcopy||mailtest
         spos=0
      end

      /* NOW locate the NEXT matching character if one                 */
      epos=pos(schar,substr(mailtest,spos+1))

      /* if epos is not last character then check if following char val*/
      if epos>0 then do
          epos=spos+epos                         /* real position      */
          if epos<length(mailtest) then do
            tchar3=substr(mailtest,epos+1,1)  
            if verify(goodpost1,tchar3,'MATCH')=0 then do
               Call TrimIt
               iterate         
            end
         end
      end
      else do  /* no matching end character */
         Call TrimIt 
         iterate         
      end

      /* OK, have a valid START and END styling character                 */ 
      if spos>1 then do 
         temp1=substr(mailtest,1,spos-1)
      end
      else do 
         temp1='' 
      end

      temp2=substr(mailtest,spos+1,epos-spos-1)

      if epos<length(mailtest) then do 
         temp3=substr(mailtest,epos+1)
      end
      else temp3=''  
      
      select
         when schar='*' then temp2=SansiBold||temp2||EansiBold
         when schar='#' then do 
            if qline & mainparm='PRINT' then do 
               temp2=EansiColr||temp2||EansiColr
            end
            else do
               temp2=SansiColr||temp2||EansiColr
            end 
         end
         when schar='_' then temp2=SansiUlin||temp2||EansiUlin
         when schar='/' then temp2=SansiItal||temp2||EansiItal
         otherwise nop
      end
      mailcopy=mailcopy||temp1||temp2
      mailtest=qattr||temp3
      spos=verify(mailtest,stylelist,'MATCH')
   end 

   maillist=qattr||mailcopy||mailtest

Return

/**************************************************************************/
/*                        Save the mail file back                         */
/**************************************************************************/
TrimIt:

   mailcopy=mailcopy||substr(mailtest,1,spos)
   mailtest=substr(mailtest,spos+1)
   if length(mailtest)>0 then do
      spos=verify(mailtest,stylelist,'MATCH')
   end
   else do
      spos=0
   end

Return

/**************************************************************************/
/*  Clear the list, reload from the OLD file, set button ghosting.        */
/**************************************************************************/
UNDO:

   button ID YTRFU ATTRS MUIA_Disabled TRUE    /* ghost UNDO button       */

   if open('IN','T:YTReForm.old','R') then do
      infotext=_text._doredo
      infobuttons=''
      showbusy=TRUE
      Call InfoWindow

      list ID YTRFM ATTRS MUIA_List_Entries
      mcnt=result
      list ID YTRFM ATTRS MUIA_List_Quiet TRUE

      method ID YTRFM MUIM_List_Clear

      do until eof('IN')
         linein=readln('IN')
         if ~eof('IN') then do
            list ID YTRFM INSERT POS MUIV_List_Insert_Bottom STRING linein
         end
      end
      foo=close('IN')
      button ID YTRFR ATTRS MUIA_Disabled FALSE   /* unghost REDO button  */
      list ID YTRFM ATTRS MUIA_List_Quiet FALSE
      foo=close('OLD')                   /* close undo file               */
      window ID YTINF close
   end
   else do
      errmsg=_text._noundofile
      Call ErrorMsg
      exit
   end


Return

/**************************************************************************/
/*  Clear the list, reload from the NEW file, set button ghosting.        */
/**************************************************************************/
REDO:

   button ID YTRFR ATTRS MUIA_Disabled TRUE    /* ghost REDO button       */

   if open('IN','T:YTReForm.new','R') then do
      infotext=_text._doredo
      infobuttons=''
      showbusy=TRUE
      Call InfoWindow

      list ID YTRFM ATTRS MUIA_List_Entries
      mcnt=result
      list ID YTRFM ATTRS MUIA_List_Quiet TRUE

      method ID YTRFM MUIM_List_Clear

      list ID YTRFM ATTRS MUIA_List_Quiet FALSE
      list ID YTRFM ATTRS MUIA_List_Quiet TRUE
      do until eof('IN')
         linein=readln('IN')
         if ~eof('IN') then do
            list ID YTRFM INSERT POS MUIV_List_Insert_Bottom STRING linein
         end
      end
      foo=close('IN')
      button ID YTRFU ATTRS MUIA_Disabled FALSE   /* unghost UNDO button  */
      list ID YTRFM ATTRS MUIA_List_Quiet FALSE
      foo=close('OLD')                   /* close undo file               */
      window ID YTINF close
   end
   else do
      errmsg=_text._noredofile
      Call ErrorMsg
      exit
   end

Return

/**************************************************************************/
/*  Clear the list, reload from the NEW file, set button ghosting.        */
/**************************************************************************/
QUIT:

   if exists('T:YTReForm.new') then do
      Address Command "Delete T:YTReForm.new QUIET"
   end

   if exists('T:YTReForm.old') then do
      Address Command "Delete T:YTReForm.old QUIET"
   end

   window ID YTREW close

Return

/**************************************************************************/
/*              Make sure YAM  and YAMTOOLS are running. Show YAM.        */
/**************************************************************************/
CheckYAM:

   if ~show('p','YAMTOOLS') then do
      errmsg=_text._noyt
      say errmsg
      exit
   end

   if ~show('p','YAM') then do
      errmsg=_text._noyam
      Call ErrorMsg
      exit
   end

   Address YAM 'show'                       /* uniconify YAM's screen     */  

	Address YAM 'info SCREEN'                /* get YAM's screen           */
	screen=result
	if screen='' then screen='Workbench'

Return

/**************************************************************************/
/*              Make sure there isn't one running already.                */
/**************************************************************************/
CheckDup:

   list ID YTRFM ATTRS MUIA_List_Entries
   if result~='RESULT' then do
      errmsg=_text._secondrun
      Call ErrorMsg
      exit
   end

Return
/******************************************************************************/
/*  Get information from YAM config file                                      */
/******************************************************************************/
GetYamInfo:

   Address YAM 'getmailinfo file'           /* find which mail to rewrap  */
   mfile=result
   
   if mfile='' | mfile='RESULT' then do     /* no mail selected           */
      errmsg=_text._nomail
      Call ErrorMsg
      exit
   end

   WordWrap=''
   QuoteText=''

   if open('IN','YAM:.config','R') then do
      do until eof('IN')
         linein=readln('IN')
         parse var linein yamopt '=' yamval
         if upper(yamopt)='WORDWRAP' then do
            WordWrap=strip(yamval)
            leave
         end
      end
      foo=close('IN')
   end

   if WordWrap='' then WordWrap=76

Return

/******************************************************************************/
/*  Display ERROR message and EXIT.                                           */
/******************************************************************************/
ErrorMsg:

   window ID YTINF CLOSE

   request ID ERRM GADGETS  _text._ok errmsg

   exit

Return

/******************************************************************************/
/*  Display the mail in a list window.                                        */
/******************************************************************************/
BuildWindow:

   if exists(UnMime.rexx) then ghostunmime=FALSE
                          else ghostunmime=TRUE

   window ID YTREW TITLE _title._main,        
          COMMAND '"YTReFormat.rexx QUIT"',
          ATTRS MUIA_Window_PublicScreen screen

		group
         group HORIZ
   			text ID YTRFF HELP help.YTRFF NODE node.YTRFF LABEL mfile
            string ID QTEXT ATTRS MUIA_ShowMe FALSE CONTENT QuoteText
            LABEL _text._wordwrap
            string ID RWRAP HELP help.RWRAP NODE node.RWRAP, 
						 ATTRS MUIA_Weight 10 CONTENT WordWrap
         endgroup
         group ATTRS MUIA_VertWeight 25
            list ID YTRFH INSERT ATTRS MUIA_ShowMe FALSE 
            list ID YTRFT COMMAND '"YTReFormat.rexx NEWHEAD %s"',
                 HELP help.YTRFT NODE node.YTRFT INSERT
         endgroup
         group HORIZ ATTRS MUIA_VertWeight 300
            group
               list ID YTRFM HELP help.YTRFM NODE node.YTRFM INSERT,
               	  ATTRS MUIA_Listview_MultiSelect MUIV_Listview_MultiSelect_Shifted
            endgroup
            group ATTRS MUIA_HorizWeight 10
               button ID YTRFW COMMAND '"YTReFormat.rexx REWRAP"',
						    HELP help.YTRFW NODE node.YTRFW, 
							 LABEL _label._YTRFW
               button ID YTRFO COMMAND '"YTReFormat.rexx UNQUOTE"', 
       				    HELP help.YTRFO NODE node.YTRFO,              
                      LABEL _label._YTRFO                           
               button ID YTRFY COMMAND '"YTReFormat.rexx STYLE"', 
						    HELP help.YTRFY NODE node.YTRFY, 
							 LABEL _label._YTRFY
               button ID YTRF3 COMMAND '"YTReFormat.rexx UNMIME"',  
	      			    HELP help.YTRF3 NODE node.YTRF3,               
                      ATTRS MUIA_Disabled ghostunmime,
                      LABEL _label._YTRF3                            
               space  
            endgroup
         endgroup
         group HORIZ
            button ID YTRFD COMMAND '"YTReFormat.rexx DONE"',
					    HELP help.YTRFD NODE node.YTRFD, 
						 LABEL _label._YTRFD
            button ID YTRFU COMMAND '"YTReFormat.rexx UNDO"', 
					    HELP help.YTRFU NODE node.YTRFU, 
                   ATTRS MUIA_Disabled TRUE,
                   LABEL _label._YTRFU
            button ID YTRFS COMMAND '"YTReFormat.rexx SAVE"', 
					    HELP help.YTRFS NODE node.YTRFS, 
						 LABEL _label._YTRFS
            button ID YTRFR COMMAND '"YTReFormat.rexx REDO"',
					    HELP help.YTRFR NODE node.YTRFR, 
                   ATTRS MUIA_Disabled TRUE,
                   LABEL _label._YTRFR
            button ID YTRFQ COMMAND '"YTReFormat.rexx QUIT"',
					    HELP help.YTRFQ NODE node.YTRFQ, 
						 LABEL _label._YTRFQ
         endgroup
		endgroup

   endwindow

Return

/******************************************************************************/
/*  Display the mail in a list window.                                        */
/******************************************************************************/
AskUser:

   OutQuote=''
   InnQuote=''

   string ID RWRAP
   WordWrap=result

   window ID YTUSR TITLE _title._user,
          COMMAND '"window ID YTUSR close"' PORT YAMTOOLS,
          ATTRS MUIA_Window_PublicScreen screen

		group HORIZ
         group 
            LABEL DOUBLE _text._outquote
            LABEL DOUBLE _text._inquote
            LABEL DOUBLE _text._wraplen
         endgroup
         group 
            string ID RUSR1 HELP help.RUSR1 NODE node.RUSR1,
                   ATTRS MUIA_CycleChain TRUE, 
                   MUIA_String_AdvanceOnCR TRUE,        
                   CONTENT OutQuote
            string ID RUSR2 HELP help.RUSR2 NODE node.RUSR2,
                   ATTRS MUIA_CycleChain TRUE, 
                   MUIA_String_AdvanceOnCR TRUE,        
                   CONTENT InnQuote
            string ID RUSR3 HELP help.RUSR3 NODE node.RUSR3,
                   ATTRS MUIA_CycleChain TRUE, 
                   MUIA_String_AdvanceOnCR TRUE,        
                   MUIA_String_Accept numerics,
                   CONTENT WordWrap
         endgroup
      endgroup
      group HORIZ
         if mainparm='REWRAP' then do
            button COMMAND '"YTReFormat.rexx DOREWRAP"' LABEL _label._YTRFW
         end
         else do
            button COMMAND '"YTReFormat.rexx DOUNQUOTE"' LABEL _label._YTRFO
         end
         button COMMAND '"window ID YTUSR close"' PORT YAMTOOLS,
                LABEL _text._cancel
		endgroup

   endwindow

   window ID YTUSR ATTRS MUIA_Window_ActiveObject MUIV_Window_ActiveObject_Next

Return

/******************************************************************************/
/*  Display Styling window                                                    */
/******************************************************************************/
StyleWin:

   window ID YTSTY TITLE _title._style,
          COMMAND '"YTReFormat.rexx STYLEIT Q"',
          ATTRS MUIA_Window_PublicScreen screen

		group HORIZ
			group
            string ID YTSTT ATTRS MUIA_ShowMe FALSE 
            list ID YTSTM HELP help.YTSTM NODE node.YTSTM INSERT,
                 ATTRS MUIA_Listview_MultiSelect MUIV_Listview_MultiSelect_Shifted
         endgroup
			group ATTRS MUIA_HorizWeight 25
            button COMMAND '"YTReFormat.rexx STYLEIT B"',
                   HELP help.YTSTB NODE node.YTSTB,
                   LABEL _label._YTSTB
            button COMMAND '"YTReFormat.rexx STYLEIT C"', 
                   HELP help.YTSTC NODE node.YTSTC,
                   LABEL _label._YTSTC
            button COMMAND '"YTReFormat.rexx STYLEIT I"', 
                   HELP help.YTSTI NODE node.YTSTI,
                   LABEL _label._YTSTI
            button COMMAND '"YTReFormat.rexx STYLEIT U"', 
                   HELP help.YTSTU NODE node.YTSTU,
                   LABEL _label._YTSTU
            button COMMAND '"YTReFormat.rexx STYLEIT R"', 
                   HELP help.YTSTR NODE node.YTSTR,
                   LABEL _label._YTSTR
            space
         endgroup
      endgroup
      group
         string ID YTST1 COMMAND '"YTReFormat.rexx STYLEIT E"',
                HELP help.YTST1 NODE node.YTST1
         text   ID YTST2 HELP help.YTST2 NODE node.YTST2
      endgroup
		group HORIZ
         button COMMAND '"YTReFormat.rexx STYLEIT S"',
                HELP help.YTSTS NODE node.YTSTS,
                LABEL _label._YTSTS
         button COMMAND '"YTReFormat.rexx STYLEIT N"', 
                HELP help.YTSTN NODE node.YTSTN,
                LABEL _label._YTSTN
         button COMMAND '"YTReFormat.rexx STYLEIT Q"',
                HELP help.YTSTQ NODE node.YTSTQ,
                LABEL _label._YTSTQ
      endgroup

   endwindow

Return

/******************************************************************************/
/*  Display Heading change window.                                            */
/******************************************************************************/
HeadWin:

   window ID YTHED TITLE _title._head,
          COMMAND '"YTReFormat.rexx HEADIT Q"',
          ATTRS MUIA_Window_PublicScreen screen

		group
         group HORIZ
            text ID YTHD0 HELP help.YTHD0 NODE node.YTHD0,
                 ATTRS MUIA_HorizWeight 25,
                 LABEL headtype
            space HORIZ
         endgroup
         group
            string ID YTHD1 HELP help.YTHD1 NODE node.YTHD1,
                   CONTENT oldhead
            string ID YTHD2 HELP help.YTHD2 NODE node.YTHD2,
                   CONTENT newhead
         endgroup
      endgroup
		group HORIZ
         button COMMAND '"YTReFormat.rexx HEADIT S"',
                LABEL _text._ok
         space HORIZ
         button COMMAND '"YTReFormat.rexx HEADIT Q"',
                LABEL _text._cancel
      endgroup

   endwindow

Return

/******************************************************************************/
/*  Simple information/error message window.                                  */
/******************************************************************************/
InfoWindow:

   window ID YTINF TITLE _title._info ATTRS MUIA_Window_PublicScreen screen,
          MUIA_Window_SizeGadget FALSE,
          MUIA_Window_DepthGadget FALSE

      group 
         group 
            text ID STEXT HELP help.STEXT  NODE node.STEXT LABEL infotext
         endgroup
         if showbusy then do
            group
               object CLASS '"Busy.mcc"' ATTRS MUIA_VertWeight 25
            endgroup
         end
         if infobuttons ~='' then do
            group HORIZ
               group 
                  space HORIZ 
               endgroup
               group 
                  radio ID SQUIT HELP help.SQUIT  NODE node.SQUIT LABELS infobuttons
               endgroup
               group 
                  space HORIZ 
               endgroup
            endgroup
         end
         else do
            group 
               space HORIZ 100
            endgroup
         end
      endgroup
   endwindow

Return

/******************************************************************************/
/*                      MUIREXX TAGS & VARIABLES                              */
/******************************************************************************/
Muivars:

MUIA_CycleChain =           0x80421ce7
MUIA_Disabled =             0x80423661
MUIA_HorizWeight =          0x80426db9
MUIA_List_Format =          0x80423c0a 
MUIA_List_Entries =         0x80421654 
MUIA_Listview_MultiSelect = 0x80427e08
MUIA_List_Quiet =           0x8042d8c7
MUIA_List_Visible =         0x8042191f
MUIA_Selected =             0x8042654b
MUIA_ShowMe =               0x80429ba8
MUIA_String_Accept =        0x8042e3e1
MUIA_VertWeight =           0x804298d0 
MUIA_Weight =               0x80421d1f 
MUIA_Window_ActiveObject =  0x80427925
MUIA_Window_DepthGadget  =  0x80421923
MUIA_Window_PublicScreen =  0x804278e4
MUIA_Window_SizeGadget  =   0x8042e33d
MUIA_String_AdvanceOnCR =   0x804226de
MUIA_Window_ActiveObject =  0x80427925

MUIM_List_Jump =            0x8042baab
MUIM_List_Clear =           0x8042ad89

TRUE=1
FALSE=0

MUIV_List_Insert_Active = -1
MUIV_List_Insert_Bottom = -3
MUIV_Listview_MultiSelect_Shifted = 2
MUIV_List_Remove_First =  0
MUIV_Window_ActiveObject_Next = -1

Return

/**************************************************************************/
/*           Various values used throughout the various routines          */
/**************************************************************************/
YTvars:

missing='.'
tab='09'x                                   /* tab character              */
numerics='"=0123456789"'                    /* valid for numeric input    */

wrapmin1=30                                 /* arbitrary low value        */
wrapmin2=20                                 /* arbitrary low value        */
wrapmax=79                                  /* max with LF for text/plain */

bo='\033b'                                  /* print control BOLD         */
bc='\033b\033c'                             /* print control BOLD CENTER  */
it='\033i'                                  /* print control ITALIC       */
ul='\033u'                                  /* print control UNDERLINED   */
co='\0335'                                  /* print control COLORED pen5 */
cx='\0332'                                  /* print control FOREGROUND   */
hl='\0338'                                  /* print control HIGHLIGHT    */
nc='\033n\033c'                             /* print control NORM CENTER  */
no='\033n'                                  /* print control NORM         */
nl='\n'                                     /* print control 1 NEW LINE   */
nl2='\n\n'                                  /* print control 2 NEW LINES  */
ct='\033c'                                  /* print control CENTER       */
lf='\033l'                                  /* print control LEFT ALIGN   */
rt='\033r'                                  /* print control RIGHT ALIGN  */

SansiBold=bo
EansiBold=no
SansiItal=it
EansiItal=no
SansiUlin=ul  
EansiUlin=no
SansiColr=co
EansiColr=cx
qattr=hl
qoff=EansiColr

stylelist='#_*/'
goodpre1=' [({\"&+'
goodpre2="'"
badpost1=' /_:;.,#+\|?=([{&%$§"!'
goodpost1=" +&'\n})]-.:,;!?"  

Return

/**************************************************************************/
/*    Messages, text, etc. constructed using previously defined values    */
/**************************************************************************/
Builtvars:

Return

/**************************************************************************/
/*               Pointers into the YamTools.guide documentation           */
/**************************************************************************/
Helpvars:

node.QTEXT='7.3.'
node.RUSR1='7.3.'
node.RUSR2='7.3.'
node.RUSR3='7.3.'
node.RWRAP='7.3.'
node.SQUIT='7.3.'
node.STEXT='7.3.'
node.YTRFD='7.3.'
node.YTRFF='7.3.'
node.YTRFM='7.3.'
node.YTRFO='7.3.'
node.YTRFQ='7.3.'
node.YTRFR='7.3.'
node.YTRFS='7.3.'
node.YTRFT='7.3.'
node.YTRFU='7.3.'
node.YTRFW='7.3.'
node.YTRFY='7.3.'
node.YTRF3='7.3.'

node.YTST1='7.3.'
node.YTST2='7.3.'
node.YTSTB='7.3.'
node.YTSTC='7.3.'
node.YTSTI='7.3.'
node.YTSTM='7.3.'
node.YTSTN='7.3.'
node.YTSTQ='7.3.'
node.YTSTR='7.3.'
node.YTSTS='7.3.'
node.YTSTU='7.3.'

node.YTHD0='7.3'
node.YTHD1='7.3'
node.YTHD2='7.3'

Return

/**************************************************************************/
/*       Mui Gadgets, text, msgs, etc. used in YamTools                   */
/**************************************************************************/
Localize:

/*********************************/
/* Miscellaneous info strings    */
/*********************************/
_title._main="""YTReFormat v1.0"""          /* main screen title          */
_title._info="""YTreFormat Info"""          /* default title on infomsg   */
_title._user="""YTReFormat User Info"""     /* title for user info screen */
_title._head="""YTreFormat Edit Header"""   /*!title for header change    */
_title._style="""YTReFormat Style"""        /* title for styling window   */

_text._ok="Ok"                              /* various OK buttons         */
_text._cancel="Cancel"                      /* LABEL for CANCEL button    */
_text._wordwrap="WordWrap"                  /* text for WordWrap field    */

_label._YTRFD="Done"                        /* label for DONE button      */
_label._YTRFU="UnDo"                        /* label for DONE button      */
_label._YTRFR="ReDo"                        /* label for DONE button      */
_label._YTRFS="Save"                        /* label for SAVE button      */
_label._YTRFQ="Quit"                        /* label for QUIT  button     */
_label._YTRFY="Style"                       /* label for STYLE button     */              
_label._YTRFO="UnQuote"                     /* label for UNQUOTE button   */
_label._YTRFW="ReWrap"                      /* label for REWRAP  button   */
_label._YTRF3="UnMime"                      /* label for UNMIME button    */

_label._YTSTB="Bold"                        /* label for BOLD    button   */
_label._YTSTC="Color"                       /* label for COLOR   button   */
_label._YTSTI="Italic"                      /* label for ITALIC  button   */
_label._YTSTU="ULine"                       /* label for ULINE   button   */
_label._YTSTR="Remove"                      /* label for REMOVE  button   */
_label._YTSTN="Skip"                        /* label for SKIP    button   */
_label._YTSTS="Save"                        /* label for SAVE    button   */
_label._YTSTQ="Quit"                        /* label for QUIT    button   */
_label._YTHDS="Save"                        /* label for SAVE    button   */
_label._YTHDQ="Quit"                        /* label for QUIT    button   */

_text._outquote="Outer Quoting:"         /* text for outer quoting prompt */
_text._inquote="Inner Quoting:"          /* text for inner quoting prompt */
_text._wraplen="Wrap Length:"            /* text for word wrap len prompt */

_text._doredo="Doing ReDo"                   /* doing redo message        */
_text._doundo="Doing UnDo"                   /* doing undo message        */
_text._dowrap="Doing ReWrapping"             /* doing ReWrapping message  */
_text._dounquote="Doing UnQuoting"           /* doing UnQuoting message   */
_text._savemail="Saving Mail File"           /* saving mailfile message   */
_text._procmail="Processing Mail for ReWrapping" /* processing mail msg   */

/*********************************/
/* Various error conditions      */
/*********************************/

_text._noyam="You need YAM running to use YAMTOOLS"    /* yam not running */
_text._noyt="You need YAMTOOLS running to use YTReFormat" /* no yamtools  */
_text._secondrun="You ALREADY have a YTReFormat started"  /* second run   */
_text._nomail="You need to select a mail file to use YTReFormat"

                                         /* not a valid file or not found */               
_text._badmail="Not a valid file selected for Rewrapping"

_text._noredofile="Unable to open REDO file"     /* bad redo file message */
_text._noundofile="Unable to open UNDO file"     /* bad undo file message */
_text._nolines="No lines selected for rewrapping" /* no lines selected    */
_text._noslines="No lines selected for styling"  /*  no lines selected    */

                  /* the word wrap is too short for the quoting requested */
_text._wwshort="Word Wrap length is too small for quoting requested"

                  /* word wrap length is out of valid range values        */
_text._wwoutrange="Word Wrap should be between "WRAPMIN1 "and " WRAPMAX

                  /* non-numeric value -- shouldn't actually happen       */
_text._wwnotnum="Word Wrap length must be a numeric value"

                  /* bad parm passed to program..should never happen      */
_text._badparm="Unrecognized parms passed to YTReFormat:"

                  /* ! message has QUOTE-PRINTABLE portions               */
_text._prtquoted="Warning! Parts of this message are in Quoted-Printable format\nYou may wish to UnMime before formatting"

/**************************************************************************/
/*           Help Messages to display with MUI bubble facility.           */
/*                                                                        */
/* Format is simple: help.ID where ID is the id specified on the MUI      */
/* object statement.                                                      */
/* Similar approach for accessing the .guide information using the NODE   */
/* option on the object statement.                                        */
/*                                                                        */
/**************************************************************************/

help.QTEXT=""""""
help.RUSR1="""Outer-most Quoting Character(s)"""
help.RUSR2="""Inner Quoting Character(s)"""
help.RUSR3="""Word Wrap length to use"""
help.RWRAP="""Word Wrap value from Yam:.config"""
help.SQUIT=""""""
help.STEXT=""""""
help.YTRFD="""Save changes to YAM mail file and Quit"""
help.YTRFF="""Yam mail file"""
help.YTRFM="""Select lines to rewrap"""
help.YTRFO="""UnQuote the selected lines"""
help.YTRFQ="""Abandon changes since last save"""
help.YTRFR="""Undo previous Undo wordwrap action"""
help.YTRFS="""Save changes to YAM mail file"""
help.YTRFT="""Short Header of mail file\nDouble-click to edit"""
help.YTRFU="""Undo previous wordwrap action"""
help.YTRFW="""ReWrap the selected lines"""
help.YTRFY="""Style the selected lines"""
help.YTRF3="""Convert mail to 8bit encoding using UnMime.rexx if available"""

help.YTST1="""Line being styled\nEdit and press RETURN"""
help.YTST2="""Line as it appears in YAM"""
help.YTSTB="""Apply BOLD styling to selected lines"""
help.YTSTC="""Apply COLOR styling to selected lines"""
help.YTSTI="""Apply ITALIC styling to selected lines"""
help.YTSTM="""Select Word(s) to Style"""
help.YTSTN="""Skip to next selected line without saving changes"""
help.YTSTQ="""Quit without saving this change"""
help.YTSTR="""Remove styling from selected lines"""
help.YTSTS="""Save changes and go to next selected line"""
help.YTSTU="""Apply UNDERLINE styling to selected lines"""

help.YTHD0="""Header type"""
help.YTHD1="""Old header"""
help.YTHD2="""Edit for new header"""

Return

