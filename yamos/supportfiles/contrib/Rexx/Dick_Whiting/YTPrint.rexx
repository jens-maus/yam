/******************************************************************************/
/*                                                                            */
/*                              YTPrint                                       */
/*                 Copyright ©1997 by Dick Whiting                            */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*    This version requires MuiRexx 3.0 use Yamtools 1.7 with MuiRexx 2.2     */
/*----------------------------------------------------------------------------*/
/* This script requires YAMTOOLS for it to work. If you don't have YAMTOOLS   */
/* you can get it on Aminet in directory comm/mail. This script allows you    */
/* to Print a Yam mail with textstyling (bold, underline, italic) as it       */
/* appears from within Yam (color changed to bold). It also allows for        */
/* saving to disk with/without converting the styling to ANSI codes.          */
/*----------------------------------------------------------------------------*/
/* To use this select a mail and decide how much you want printed, saved, or  */
/* viewed and whether to STYLE or not. There is also an option to print/save  */
/* each section of the mail as a separate page/file.                          */
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
/*                           21 June 1997                                     */
/*                                                                            */
/******************************************************************************/
/*
$VER: 1.0 Copyright ©1997 by Dick Whiting
$AUTHOR: Dick Whiting
$DESCRIPTION: Print a mail file with styling     
*/

options results
options failat 21

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
      Call BuildWindow
      Call IdMail
   end
   when mainparm='READ' then do
      Call ViewPrint  
   end
   when mainparm='VIEW' then do
      Call ViewPrint
   end
   when mainparm='PRINT' then do
      Call ViewPrint
   end
   when mainparm='DISK' then do
      Call ViewPrint
   end
   when mainparm='UNMIME' then do
      Call GetYamInfo
      method ID YTPRM MUIM_List_Clear
      method ID YTPRH MUIM_List_Clear
      method ID YTPRT MUIM_List_Clear
      method ID YTPRR MUIM_List_Clear
      Call UnMime
      Call ReadMail
      Call IdMail
   end
   when mainparm='QUIT' then do
      'window ID YTPRW close'
   end
   otherwise do
      errmsg=_text._badparm
      Call ErrorMsg
   end
end 

j=0
k=j
do i=1 to selected.0
   list ID YTPRM ATTRS MUIA_List_Entries
   mcnt=result
   do j=k to mcnt-1
      list ID YTPRM POS j
      xsubparm=result
      if xsubparm=selected.i then do
         reselptr=1+reselect.0
         reselect.0=reselptr
         reselect.reselptr=j
         k=j
         leave j
      end
   end
end
do j=1 to reselect.0
   method ID YTPRM MUIM_List_Select reselect.j MUIV_List_Select_On
end

exit

/**************************************************************************/
/*                        Read the mail file into array                   */
/**************************************************************************/
ReadMail:     

   maillist.=''
   maillist.0=0
   i=0

   if open('IN',mfile,'R') then do
      do while ~eof('IN')
         linein=readln('IN')
         if ~eof('IN') then do
            i=i+1
            linein=translate(linein,' ',tab)
            maillist.i=linein
         end
      end
      maillist.0=i
      foo=close('IN')
      window ID YTINF close
   end
   else do
      errmsg=_text._badmail
      Call ErrorMsg
      exit     
   end

Return

/**************************************************************************/
/*                 Loop thru the mail identifying parts                   */
/**************************************************************************/
IdMail:     

   infotext=_text._prepmail
   infobuttons=''
   showbusy=TRUE
   Call InfoWindow

	header.=' '

   parts.0=0
   parts.=''

   i=0
   header=TRUE
   multipart=FALSE
   quoted=FALSE
   bfound=FALSE   
   parttype='H'
   boundary=''
   partname='text/plain'
   content='text/plain'
   encoding=''
   partnum=0
   headnum=1

   list ID YTPRT ATTRS MUIA_List_Quiet TRUE 
   list ID YTPRM ATTRS MUIA_List_Quiet TRUE 

   do i=1 to maillist.0
      if header then do
         if maillist.i~='' then do
				select
		         when upper(word(maillist.i,1))='FROM:' then header.1=maillist.i
		         when upper(word(maillist.i,1))='TO:'   then header.2=maillist.i
		         when upper(word(maillist.i,1))='DATE:' then header.3=maillist.i
		         when upper(word(maillist.i,1))='SUBJECT:' then header.4=maillist.i
		         when upper(word(maillist.i,1))='CONTENT-TYPE:' then do
                  if pos('BOUNDARY=',upper(maillist.i))>0 then do
                     parse var maillist.i junk '=' boundary
                     boundary=strip(boundary,'B','"')
                     if boundary~='' then do 
                        boundary='--'||boundary
                        endbound=boundary||'--'
                        multipart=TRUE
                     end
                  end
                  else do
                     content=word(maillist.i,2)
                     content=strip(content,'T',';')
                     parse var maillist.i junk'name='partname';'rest
                     partname=strip(partname,'B','"')
                  end
               end
		         when upper(word(maillist.i,1))='CONTENT-TRANSFER-ENCODING:' then do
                  encoding=word(maillist.i,2)
                  if upper(encoding)='QUOTED-PRINTABLE' then do 
                     quoted=TRUE
                  end
               end
					otherwise nop
				end
            call LoadParts
         end           
         else do  /* start of section after a header */
            partnum=partnum+1
            if multipart & ~bfound then parttype='L' 
                                   else parttype='B'
            call LoadParts
            headnum=headnum+1
            header=FALSE
         end
      end   
      else do
         if ~multipart then do
            call LoadParts
         end
         else do  /* need to handle multipart */
            select
               when ~bfound & maillist.i~=boundary then do
                  call LoadParts
               end
               when ~bfound & maillist.i=boundary then do
                  parttype='X'
                  call LoadParts
                  bfound=TRUE
                  partcnt=partcnt+1
                  j=i+1
                  if maillist.j='' then do /* line after boundary null */
                     parttype='B'       /*!changed from T */
                     headnum=headnum+1  /* inserted       */
                  end
                  else do /* line after boundary NOT null */
                     parttype='H'
                     header=TRUE
                  end
               end
               when bfound & maillist.i=endbound then do
                  parttype='X'
                  call LoadParts
                  partcnt=partcnt+1
                  parttype='T' 
               end
               when bfound & maillist.i~=boundary then do
                  call LoadParts
               end
               when bfound & maillist.i=boundary then do
                  parttype='X'
                  call LoadParts
                  bfound=TRUE
                  j=i+1
                  if maillist.j='' then do /* line after boundary null */
                     parttype='B' 
                     headnum=headnum+1
                     partcnt=partcnt+1
                  end
                  else do /* line after boundary NOT null */
                     parttype='H'
                     header=TRUE
                  end
               end
               otherwise do
                  say 'found otherwise in IdMail'
               end
            end   
         end 
      end
   end

	do j=1 to 4
	   list ID YTPRT INSERT POS MUIV_List_Insert_Bottom STRING '='header.j
	end

   parttype=parts.1
   partcnt=parts.1.1
   sectcnt=1

   do i=2 to maillist.0
      if parts.i=parttype & parts.i.1=partcnt then do
         sectcnt=sectcnt+1
      end          
      else do
         Call ReportParts
         parttype=parts.i
         partcnt=parts.i.1
         sectcnt=1
      end
   end

   i=maillist.0
   Call ReportParts

   list ID YTPRT ATTRS MUIA_List_Quiet FALSE 
   list ID YTPRM ATTRS MUIA_List_Quiet FALSE 

   list ID YTPRM ATTRS MUIA_List_Entries
   mcnt=result

   do i=0 to mcnt-1
      list ID YTPRM POS i
      subparm=result
      parse var subparm parttype ',' xpartcnt ',' rest
      if substr(parttype,3)='Body' then leave i
   end

   if i>0 then do
      firsttime=TRUE
      method ID YTPRM MUIM_List_Select i MUIV_List_Select_On
      mainparm='VIEW'
      Call ViewPrint
   end

   if quoted then do
      errmsg=_text._prtquoted
      Call ErrorMsg
      exit
   end

Return

/**************************************************************************/
/*             Build array with type information for each line            */
/**************************************************************************/
LoadParts:

   if parttype='H' then partcnt=headnum
                   else partcnt=partcnt

   linein=parttype||','||partcnt||','||partname||','||content||','||encoding||',='||maillist.i
   list ID YTPRH INSERT POS MUIV_List_Insert_Bottom, 
        ATTRS MUIA_List_Format """,,,,,""",
        STRING linein
   parts.i=parttype
   parts.i.1=partcnt
   parts.i.1.1=partname
   parts.i.1.1.1=content
   parts.i.1.1.1.1=encoding

Return

/**************************************************************************/
/*             Display one line per portion of Mail File                  */
/*             !!! need to figure out what is encodet how in multipart    */
/**************************************************************************/
ReportParts:

   if parttype~='B' then do
      partname=''
      content=''
      encoding=''
   end
   else do
      partname=parts.i.1.1
      content=parts.i.1.1.1
      encoding=parts.i.1.1.1.1
   end

   select
      when parttype='H' then parttype='Header'
      when parttype='L' then parttype='Leader'
      when parttype='X' then parttype='Boundary'
      when parttype='B' then parttype=bo||'Body'
      when parttype='T' then parttype='Trailer'
      otherwise parttype='Unknown'
   end

   linein=parttype||','||partcnt||','||sectcnt||','||partname||','||content||','||encoding
   list ID YTPRM INSERT POS MUIV_List_Insert_Bottom,  
        ATTRS MUIA_List_Format """P=\033l BAR,P=\033r BAR,P=\033r BAR,P=\033l BAR,BAR,""", 
        STRING linein

Return

/**************************************************************************/
/*                    Display portion of mail in list                     */
/**************************************************************************/
DisplayMail:

   parse var subparm parttype ',' xpartcnt ',' rest

   list ID YTPRM ATTRS MUIA_List_Entries
   mcnt=result

   do i=0 to mcnt-1
      list ID YTPRM POS i
      xsubparm=result
      if xsubparm=subparm then do
         selected.0=1
         selected.1=subparm
         leave i
      end
   end

   parttype=substr(parttype,2)
   if c2d(substr(parttype,1,1))=27 then do
      parttype=substr(parttype,3)
   end
   select
      when parttype='Header'   then xparttype='H'
      when parttype='Leader'   then xparttype='L'
      when parttype='Boundary' then xparttype='X'
      when parttype='Body'     then xparttype='B'
      when parttype='Trailer'  then xparttype='T'
      otherwise do
         errmsg=_text._badparttype
         Call ErrorMsg
         exit
      end
   end

   list ID YTPRH ATTRS MUIA_List_Entries
   mcnt=result

   list ID YTPRR ATTRS MUIA_List_Quiet TRUE  
   method ID YTPRR MUIM_List_Clear

   found=FALSE
   do i=0 to mcnt-1
      list ID YTPRH POS i
      linein=result
      parse var linein parttype ',' partcnt ',' partname ',' content ',' encoding ',' maillist
      if xparttype=parttype & xpartcnt=partcnt then do
         found=TRUE
         list ID YTPRR INSERT POS MUIV_List_Insert_Bottom,
         STRING '='maillist
      end
      else do
         if found then leave
      end
   end 
   list ID YTPRR ATTRS MUIA_List_Quiet FALSE

Return

/**************************************************************************/
/*             Main Logic Loop for viewing and printing                   */
/**************************************************************************/
ViewPrint:

   if ~firsttime then do
      infotext=_text._prepmail
      infobuttons=''
      showbusy=TRUE
      Call InfoWindow 
   end

   cycle  ID YTPR4
   styled=result
   if styled=_label._ytpr4a then styled=TRUE
                            else styled=FALSE

   cycle  ID YTPR5
   multijob=result
   if multijob=_label._ytpr5b then multijob=TRUE
                              else multijob=FALSE

   cycle  ID YTPR7
   coloropt=result

   if coloropt=_label._ytpr7a then coloropt=FALSE
                              else coloropt=TRUE

   Call StyleVals

   select
      when mainparm='VIEW' then do
         list ID YTPRR ATTRS MUIA_List_Quiet TRUE 
         method ID YTPRR MUIM_List_Clear
         Call DoViewPrint
         list ID YTPRR ATTRS MUIA_List_Quiet FALSE
      end
      when mainparm='PRINT' then do
         if open('OUT','PRT:','W') then do
            Call DoViewPrint
            foo=close('OUT')
         end  /* good open on PRT: */
         else do
            errmsg=_text._noprinter
            Call ErrorMsg
            exit
         end
      end
      otherwise do /* better be SAVE to DISK */
         Call GetFileName
         Call DoViewPrint
      end
   end

   window ID YTINF close

Return

/**************************************************************************/
/*              Handle looping thru selected mail parts                   */
/**************************************************************************/
DoViewPrint:
     
   list ID YTPRM ATTRS MUIA_List_Entries
   mcnt=result

   list ID YTPRH ATTRS MUIA_List_Entries
   hcnt=result

   do i=0 to mcnt-1
      list ID YTPRM
      subparm=result
      if subparm='' then leave i
      selptr=1+selected.0
      selected.0=selptr
      selected.selptr=subparm
      parse var subparm parttype ',' xpartcnt ',' rest

      if c2d(substr(parttype,1,1))=27 then do
         parttype=substr(parttype,3)
      end

      select
         when parttype='Header'   then xparttype='H'
         when parttype='Leader'   then xparttype='L'
         when parttype='Boundary' then xparttype='X'
         when parttype='Body'     then xparttype='B'
         when parttype='Trailer'  then xparttype='T'
         otherwise do
            errmsg=_text._badparttype
            Call ErrorMsg
            exit
         end
      end

      found=FALSE

      if mainparm='PRINT' & multijob & i>0 then do 
         foo=writeln('OUT',FF)
      end
      if mainparm='DISK' & multijob & i>0 then do 
         foo=close('OUT')
         Call GetFileName
      end

      if i=0 then do
         if coloropt & styled & mainparm='PRINT' then do
            foo=writech('OUT',qoff)          /* force to normal text      */ 
         end
         if xparttype~='H' | xpartcnt~=1 then do
            Call DoHeader
         end
      end

      do j=0 to hcnt-1
         list ID YTPRH POS j
         linein=result
         parse var linein parttype ',' partcnt ',' partname ',' content ',' encoding ',' maillist
         if xparttype=parttype & xpartcnt=partcnt then do
            found=TRUE
            if styled then do
               Call StyleIt
            end
            if mainparm='VIEW' then do
               maillist=translate(maillist,' ',',')
               if maillist='' then maillist=no
               list ID YTPRR INSERT POS MUIV_List_Insert_Bottom,
               STRING maillist
            end
            else do  /* Print OR Save it */
               foo=writeln('OUT',maillist)
            end
         end
         else do
            if found then leave j
         end 
      end  
   end

   if i=0 then do
      errmsg=_text._nosect
      Call ErrorMsg
      exit
   end

Return

/**************************************************************************/
/*                   Handle type of header to include                     */
/**************************************************************************/
DoHeader:

   cycle  ID YTPR6
   headopt=result

   select
      when mainparm='VIEW' then nop          /* no headers on VIEW option */
      when headopt=_label._ytpr6c then nop   /* set to NONE for header    */
      when headopt=_label._ytpr6b then do    /* set to FULL for header    */
         do j=0 to hcnt-1                    /* loop thru file            */
            list ID YTPRH POS j
            linein=result
            parse var linein parttype ',' partcnt ',' partname ',' content ',' encoding ',' maillist
            if parttype='H' & partcnt=1 then do
               foo=writeln('OUT',maillist)  
            end
            else leave j
         end
      end
      otherwise do                           /* set to SHORT for header   */
         do j=0 to 3
            list ID YTPRT POS j
            headline=result
            foo=writeln('OUT',headline)
         end
      end
   end

Return

/**************************************************************************/
/*                        Get Filename for SAVES                          */
/**************************************************************************/
GetFileName:

   getvar YTPRINTDIR 
   ytprintfile=result   

   select
      when lastpos('/',ytprintfile)>0 then do
         savfile=substr(ytprintfile,lastpos('/',ytprintfile)+1)
         savdir=substr(ytprintfile,1,lastpos('/',ytprintfile)-1)
      end
      when lastpos(':',ytprintfile)>0 then do
         savfile=substr(ytprintfile,lastpos(':',ytprintfile)+1)
         savdir=substr(ytprintfile,1,lastpos(':',ytprintfile))
      end
      otherwise do
         savfile=''
         savdir="""T:"""
      end
   end

   aslrequest ID YTPRW TITLE _title._asl,
              ATTRS ASLFR_InitialDrawer savdir ASLFR_InitialFile savfile 
   if rc = 0 then do
      ytprintfile=result
      setvar YTPRINTDIR ytprintfile
      if ~open('OUT',ytprintfile,'W') then do
         errmsg=_text._badsavefile
         Call ErrorMsg
         exit
      end
   end
   else do
      window ID YTINF close
      exit
   end

Return

/**************************************************************************/
/*                        Set Styling code values                         */
/*                                                                        */
/**************************************************************************/
StyleVals:

   stylelist='#_*/'
   goodpre1=' [({\"&+'
   goodpre2="'"
   badpost1=' /_:;.,#+\|?=([{&%$§"!'
   goodpost1=" +&'\n})]-.:,;!?"  

   select
      when mainparm='VIEW' then do    /* show with MUI formatted styling  */
         SansiBold=bo
         EansiBold=no
         SansiItal=it
         EansiItal=no
         SansiUlin=ul  
         EansiUlin=no
         SansiColr=co
         EansiColr=cx
         QuoteColr=hl
         qattr=QuoteColr
         qoff=EansiColr
      end
      when coloropt & mainparm='DISK' then do  /* save as COLORED styling */
         csi='1B5B'x
         SansiBold=csi||'1m'
         EansiBold=csi||'22m'
         SansiItal=csi||'3m'
         EansiItal=csi||'23m'
         SansiUlin=csi||'4m'
         EansiUlin=csi||'24m'
         SansiColr=csi||'33;40m'
         EansiColr=csi||'31;40m'
         QuoteColr=csi||'37;40m'
         qattr=QuoteColr
         qoff=EansiColr
      end
      when coloropt & mainparm='PRINT' then do /* print as COLORED styling */
         csi='1B5B'x
         SansiBold=csi||'1m'
         EansiBold=csi||'22m'
         SansiItal=csi||'3m'
         EansiItal=csi||'23m'
         SansiUlin=csi||'4m'
         EansiUlin=csi||'24m'
         SansiColr=csi||'36;47m'
         EansiColr=csi||'30;47m'
         QuoteColr=csi||'31;47m'
         qattr=QuoteColr
         qoff=EansiColr
      end
      otherwise do                    /* print or save as B/W styling     */
         csi='1B5B'x
         SansiBold=csi||'1m'
         EansiBold=csi||'22m'
         SansiItal=csi||'3m'
         EansiItal=csi||'23m'
         SansiUlin=csi||'4m'
         EansiUlin=csi||'24m'
         SansiColr=csi||'1m'
         EansiColr=csi||'22m'
         QuoteColr=SansiColr
         qattr=QuoteColr
         qoff=EansiColr
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
StyleIt:     

   qattr=QuoteColr
   qline=TRUE
   testword=word(maillist,1)
   if pos('>',testword)=0 then do 
      qline=FALSE
      qattr=qoff
   end

/******************* better logic for quoted lines **********
   qline=TRUE
   testword=word(maillist,1)
   if length(testword)>0 then do 
      tchar0=substr(testword,length(testword),1)
      if tchar0~='>' & substr(testword,1,1)~='>' then do 
         qline=FALSE
         qattr=qoff
      end
   end
   else do
      qline=FALSE
      qattr=qoff
   end
********************************************************/

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

   list ID YTPRM ATTRS MUIA_List_Entries
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

   window ID YTPRW TITLE _title._main,        
          COMMAND '"YTPrint.rexx QUIT"',
          ATTRS MUIA_Window_PublicScreen screen

		group
         group 
   			text ID YTPRF HELP help.YTPRF NODE node.YTPRF LABEL mfile
            list ID YTPRH INSERT ATTRS MUIA_ShowMe FALSE 
         endgroup
         group HORIZ ATTRS MUIA_VertWeight 100
            group 
               list ID YTPRT HELP help.YTPRT NODE node.YTPRT INSERT,
                    ATTRS MUIA_VertWeight 70 
               list ID YTPRM HELP help.YTPRM NODE node.YTPRM INSERT,
                    COMMAND '"YTPrint.rexx VIEW %s"',
                    TITLE _title._ytprm,
               	  ATTRS MUIA_Listview_MultiSelect MUIV_Listview_MultiSelect_Shifted
            endgroup
            group HORIZ ATTRS MUIA_HorizWeight 20
               group
                  cycle  ID YTPR4 HELP help.YTPR4 NODE node.YTPR4,
                         LABELS _label._ytpr4
                  cycle  ID YTPR5 HELP help.YTPR5 NODE node.YTPR5,
                         LABELS _label._ytpr5
                  cycle  ID YTPR6 HELP help.YTPR6 NODE node.YTPR6,
                         LABELS _label._ytpr6
                  cycle  ID YTPR7 HELP help.YTPR7 NODE node.YTPR7,
                         LABELS _label._ytpr7
                  space 
               endgroup
               group
                  button ID YTPR0 COMMAND '"YTPrint.rexx VIEW"',
							    HELP help.YTPR0 NODE node.YTPR0, 
								 LABEL _label._YTPR0
                  button ID YTPR1 COMMAND '"YTPrint.rexx PRINT"',
							    HELP help.YTPR1 NODE node.YTPR1, 
								 LABEL _label._YTPR1
                  button ID YTPR2 COMMAND '"YTPrint.rexx DISK"',  
		      			    HELP help.YTPR2 NODE node.YTPR2,               
                         LABEL _label._YTPR2                            
                  button ID YTPR3 COMMAND '"YTPrint.rexx UNMIME"',  
		      			    HELP help.YTPR3 NODE node.YTPR3,               
                         ATTRS MUIA_Disabled ghostunmime,
                         LABEL _label._YTPR3                            
                  space
                  button ID YTPRQ COMMAND '"YTPrint.rexx QUIT"',
							    HELP help.YTPRQ NODE node.YTPRQ, 
								 LABEL _label._YTPRQ
               endgroup
            endgroup
         endgroup
         group ATTRS MUIA_VertWeight 150
            list ID YTPRR HELP help.YTPRR NODE node.YTPRR INSERT
         endgroup
		endgroup

   endwindow

Return

/******************************************************************************/
/*  Simple information/error message window.                                  */
/******************************************************************************/
InfoWindow:

   window ID YTINF TITLE _title._info ATTRS MUIA_Window_PublicScreen screen,
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

/*
MUIA_CycleChain =           0x80421ce7
MUIA_String_Accept =        0x8042e3e1
MUIA_Window_ActiveObject =  0x80427925
MUIA_Window_ActiveObject =  0x80427925
MUIA_String_AdvanceOnCR =   0x804226de
MUIA_Window_SizeGadget  =   0x8042e33d
MUIM_List_Jump =            0x8042baab
MUIV_List_Insert_Active = -1
MUIV_List_Remove_First =  0
MUIV_Window_ActiveObject_Next = -1
*/

MUIA_Disabled =             0x80423661
MUIA_HorizWeight =          0x80426db9
MUIA_List_Active =          0x8042391c 
MUIA_List_Format =          0x80423c0a 
MUIA_List_Entries =         0x80421654 
MUIA_Listview_MultiSelect = 0x80427e08
MUIA_List_Quiet =           0x8042d8c7
MUIA_List_Visible =         0x8042191f
MUIA_Selected =             0x8042654b
MUIA_ShowMe =               0x80429ba8
MUIA_VertWeight =           0x804298d0 
MUIA_Weight =               0x80421d1f 
MUIA_Window_DepthGadget  =  0x80421923
MUIA_Window_PublicScreen =  0x804278e4

ASLFR_InitialFile =         0x80080008
ASLFR_InitialDrawer =       0x80080009

MUIM_List_Clear =           0x8042ad89
MUIM_List_Select =          0x804252d8 

TRUE=1
FALSE=0

MUIV_List_Insert_Bottom = -3
MUIV_List_Select_Off = 0
MUIV_List_Select_On = 1
MUIV_Listview_MultiSelect_Shifted = 2

Return

/**************************************************************************/
/*           Various values used throughout the various routines          */
/**************************************************************************/
YTvars:

missing='.'
comma=','
tab='09'x                                   /* tab character              */
firsttime=FALSE
selected.0=0
reselect.0=0

FF='0C'x                                    /* print control FORM FEED    */
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

Return

/**************************************************************************/
/*    Messages, text, etc. constructed using previously defined values    */
/**************************************************************************/
Builtvars:

_title._ytprm='"'||bc||_col._ytprm1||comma||bo||_col._ytprm2||comma||bo||_col._ytprm3||comma||bc||_col._ytprm4||comma||bo||_col._ytprm5||comma||bo||_col._ytprm6||'"'
_label._ytpr4=_label._ytpr4a||comma||_label._ytpr4b
_label._ytpr5=_label._ytpr5a||comma||_label._ytpr5b
_label._ytpr6=_label._ytpr6a||comma||_label._ytpr6b||comma||_label._ytpr6c
_label._ytpr7=_label._ytpr7a||comma||_label._ytpr7b

Return

/**************************************************************************/
/*               Pointers into the YamTools.guide documentation           */
/**************************************************************************/
Helpvars:

node.SQUIT='7.3.'
node.STEXT='7.3.'
node.YTPRF='7.3.'
node.YTPRT='7.3.'
node.YTPRM='7.3.'
node.YTPRR='7.3.'
node.YTPR0='7.3.'
node.YTPR1='7.3.'
node.YTPR2='7.3.'
node.YTPR3='7.3.'
node.YTPR4='7.3.'
node.YTPR5='7.3.'
node.YTPR6='7.3.'
node.YTPR7='7.3.'
node.YTPRQ='7.3.'

Return

/**************************************************************************/
/*       Mui Gadgets, text, msgs, etc. used in YamTools                   */
/**************************************************************************/
Localize:

/*********************************/
/* Miscellaneous info strings    */
/*********************************/
_title._main="""YTPrint v1.0"""            /* main screen title          */
_title._info="""YTPrint Info"""            /* default title on infomsg   */
_title._asl="""Select File"""              /* title for ASL requester    */ 

_text._ok="Ok"                              /* various OK buttons         */
_text._cancel="Cancel"                      /* LABEL for CANCEL button    */

_label._ytpr0="View"                        /* label for VIEW button      */
_label._ytpr1="Print"                       /* label for Print button     */
_label._ytpr2="Disk"                        /* label for DISK button      */
_label._ytpr3="UnMime"                      /* label for UNMIME button    */
_label._ytpr4a="Styled"                     /* convert to ANSI text style */ 
_label._ytpr4b="Normal"                     /* leave text as is           */
_label._ytpr5a="Single"                     /* process as single file     */
_label._ytpr5b="Multiple"                   /* treat each section separate*/

_label._ytpr6a="Short"                      /* type of header to include  */
_label._ytpr6b="Full"                       /* type of header to include  */
_label._ytpr6c="None"                       /* type of header to include  */

_label._ytpr7a="B/W"                        /* format for black and white */
_label._ytpr7b="Color"                      /* format for color printing  */

_label._ytprq="Quit"                        /* label for QUIT  button     */

_col._ytprm1="Type"                         /* heading for ytprm column 1 */
_col._ytprm2="N"                            /* heading for ytprm column 2 */
_col._ytprm3="Lines"                        /* heading for ytprm column 3 */
_col._ytprm4="Name"                         /* heading for ytprm column 1 */
_col._ytprm5="Content"                      /* heading for ytprm column 2 */
_col._ytprm6="Encoding"                     /* heading for ytprm column 3 */

_text._savemail="Saving Mail File..."        /* saving mailfile message   */
_text._prepmail="Preparing Mail..." /* preparing mail message   */
_text._noprinter="Unable to open PRT:\nIs the Printer on?"

/*********************************/
/* Various error conditions      */
/*********************************/

_text._noyam="You need YAM running to use YAMTOOLS"    /* yam not running */
_text._noyt="You need YAMTOOLS running to use YTPrint" /* no yamtools  */
_text._secondrun="You ALREADY have a YTPrint started"  /* second run   */
_text._nomail="You need to select a mail file to use YTPrint"
_text._nosect="You need to select mail section(s) to View/Print/Save"
_text._badmail="Cannot open the mail file"
_text._badparm="Unrecognized parms passed to YTPrint:"
_text._prtquoted="Warning! Parts of this message are in Quoted-Printable format\nYou may wish to UnMime before printing"
_text._badparttype="Found unknown part type in mail"
_text._badsavefile="Unable to open output file"

/**************************************************************************/
/*           Help Messages to display with MUI bubble facility.           */
/*                                                                        */
/* Format is simple: help.ID where ID is the id specified on the MUI      */
/* object statement.                                                      */
/* Similar approach for accessing the .guide information using the NODE   */
/* option on the object statement.                                        */
/*                                                                        */
/**************************************************************************/

help.SQUIT=""""""
help.STEXT=""""""
help.YTPRF="""Yam mail file"""
help.YTPRT="""Short Header of mail file"""
help.YTPRM="""Information about each mail section\nDouble-click to Read section\nMulti-select to View/Print/Save"""
help.YTPRR="""Area for Reading/Viewing mail sections"""
help.YTPR0="""View selected section(s) in read area"""
help.YTPR1="""Print selected section(s)\nEnsure printer is ON"""
help.YTPR2="""Save selected section(s) to disk"""
help.YTPR3="""Convert mail to 8bit encoding using UnMime.rexx if available"""
help.YTPR4="""Leave as plain text\nConvert to ANSI text styling"""
help.YTPR5="""Process selected section(s) as single file\nProcess each section separate"""
help.YTPR6="""Type of header to include in Printing/Saving to disk\nIf the Header is not selected"""
help.YTPR7="""Styled for Black and White or for COLOR\non PRINT or save to DISK"""
help.YTPRQ="""Quit"""

Return


