/***
*    $VER: YamToGuide  v1.1ß (12.08.98) ©Scott Beardwood
*
*
*  Modified from the original at the request of Scott Beardwood
*         by Chris Eburn <ceburn@midcoast.com.au>
*
*  There are two simple ways to use this script (and several more less simple)
*
*  1) rx YamToGuide.rexx [[+f]FolderId] [[+t]GuideTitle]
*  2) from the Yam rexx menu  <-- see the YAM manual for this optimal technique
*
*  The two variables of FolderId and GuideTitle are optional.
*  Either position OR plus commands (+lowercase) may be used
*      to specify which variable[s] have been given.
*
*  If no Folder or Title is supplied then the current folder and a guide
*      with a title based on that folder name will be processed
*
*
*     This script was developed on an A2000/030 25Mhz machine with 9Megs RAM
*  and as such the fully commented nature has little affect on execution speed
*     a reduction in loading time would be achieved by removing comment BUT
*         this will NOT increase the speed of execution once running
*
*<---The text formating for hackers may be problomatic as I use a wide display-------->
*
* Any questions about this script may be directed to either myself, Chris
* Eburn (ceburn@midcoast.com.au>, or Scott Beardwood <scott@online.u-net.com>.
*
* The copyright for this script belongs to Scott Beardwood who had the
* original idea/need and created the original logic flow. I retain the
*     right to use individual program components as I see fit.
*
*    THIS IS A LARGLY UNTESTED BETA RELEASE, USE IT AT YOUR OWN RISK.
***/

Output_dir  = 'ram:'  /* where do you want the finished guide  */
                      /* this path must end with a ':' or '/'  */

/*******************************************************/

if index(arg,'+') = 0 then parse arg Folder_id Guide_title
else parse arg '+f' Folder_id '+' . ,1 '+t' Guide_title '+' .


/* Folder_id = 6  Guide_title = '' original test settings */

/*       Possible folder_id's       NB: Id may be number or name         */
/* 0 Incoming ; 1 Outgoing ; 2 Sent ; 3 Archived ; 4 - 51 User Defined   */
/* For your user defined folders use names or check the YAM:.CONFIG file */

options results

Temp_header = 't:Header.ytg'     /* temporary file paths   */
Temp_main   = 't:Main.ytg'       /* somewhere fast is nice */

nl    = '0a'x ; tab = '09'x      /* system definitions */
star  = '@'   ; Info_rate = 25   /* user display mark ; display update rate, larger numbers give a slower rate */
width = 75                       /* caracters per line for header info */

call open('window','con:40/40/400/120/ Yam To Guide/close')   /* user display */

call writeln('window','')
call writeln('window',tab 'Yam Mail To Amiga Guide'  )
call writeln('window',tab '© Scott Beardwood 1998' nl)
call writeln('window',tab '     Please Wait'       nl)

if ~show(p,'YAM') then call err('Yam port')

address 'YAM'
'show'                                      /* Yam makes a nice active display */

if Folder_id ~= '' then 'setfolder' Folder_id

'getfolderinfo name'
   FolderName = RESULT

if Guide_title = '' then Guide_title = 'Yam_'FolderName'.guide'   /* set default guide name */

if index(':',Guide_title) = 0 then Guide_name = Output_dir || Guide_title  /* and append output path if required */
   else Guide_name = Guide_title     /* we recieved a fully qualified name on the command line */

'getfolderinfo max'
   MailCount = RESULT
   If MailCount <= 0 then call err('empty folder  ('FolderName')')
   else MailCount = MailCount - 1

if open('header',Temp_header,'w') then do
   if open('main',Temp_main ,'w') then do

      do Idx = 0 to MailCount
         'setmail' Idx
         'getmailinfo from'    ; parse var RESULT From '<' .
         'getmailinfo subject' ; Subject = RESULT
         'getmailinfo file'    ; Path    = RESULT

         LinkName = 'Mail_'idx

         call writeln('header','@{" 'Subject' " Link  "'LinkName'"}' tab From)
         call writeln('main'  ,'@NODE "'LinkName'" "'Subject'"')

         Date.Idx = writemail()          /* Date.x appears in the header */

         call writeln('main','@ENDNODE')
      end

   end      /* if open(main) do */
   else call err(Temp_main '(w)')

   call writeln('header','@ENDNODE')

   call close('main')
   call close('header')

end      /* if open(header) do */
else call err(Temp_header '(w)')

call MakeGuide()                   /* get all the bits together as the final guide */

address command 'c:delete' Temp_header Temp_main 'quiet'           /* tidy up mess */

call err('1B5B4D'x nl tab Guide_name)  /* '1B5B4D'x is delete current line "esc[M" */
exit                                   /* normal end of program */

/********************************************************/

writemail:
/*    Note: Anything in the Drop variable as lowercase will be kept    */

Drop = 'ANNOUCE CC content-transfer-encoding CONTENT-TYPE date ,
        DELIVERED-TO DELIVERY-DATE ENVELOPE-TO from IN-REPLY-TO MAILING-LIST ,
        MESSAGE-ID MIME-VERSION organization PRIORITY RECEIVED REFERENCES ,
        REPLY-TO RETURN-PATH subject TO X-MAILER X-MIME-AUTOCONVERTED X-MIMEOLE ,
        X-MSMAIL-PRIORITY X-POP3-RCPT X-PRIORITY X-SENDER'

   if ~open('letter',Path,'r') then return 'date unknown'          /* for header */

   do idy = 1 until eof('letter')
      if (idy//Info_Rate) = 0 then call writech('window',star) /* update user display */
   
      inLine = readln('letter')
      parse upper var inLine ChkWord ':' .     /* catch transmission header keywords */
   
      do while (ChkWord = 'RECEIVED')&(find(Drop,'received') = 0) /* drop RECEIVED info */
         do for 3
            inLine = translate( readln('letter') ,,tab)    /* turn tabs into spaces */
            parse var InLine Extra .
            if find('id by for',Extra) = 0 then leave   /* recieved: info may be spread over several lines */
         end
         parse upper var inLine ChkWord ':' .  /* recover from recieved: loop before more line checks */
      end

      if ChkWord = 'DATE' then parse var inline . Wkday Day Mth Yr .
      if ChkWord = 'CONTENT-TRANSFER-ENCODING' then parse var inLine . '-' . '-' inLine


      if (find(Drop,ChkWord) = 0)|(inLine = '') then call writeln('main',inLine)  /* if its not an excluded line then add it to the guide */
   end                                                                            /* we need to check blanklines as FIND() wont           */

   call close('letter')                                  /* added one mail item */
return Wkday Day'/'Mth'/'Yr                        /* send back the date for header */

/********************************************************/

makeguide:
   if ~open('guide',Guide_name,'w') then call err(Guide_name '(w)')

       /* write the standard and required header lines */

   call writeln('guide','@DATABASE "'Guide_title'"')
   call writeln('guide','@$VER:' date() time())
   call writeln('guide','@WORDWRAP')
   call writeln('guide','@REM Created with YamToGuide.rexx  ©Scott Beardwood' nl)
   call writeln('guide','@NODE INFO "'Information'"' nl)
   call writeln('guide',centre(Guide_title tab date(),width) nl)
   call writeln('guide',centre('contains Yam mail within the following dates',width))
   call writeln('guide',centre('first:' Date.0 '  last:' Date.MailCount,width) nl)

       /* write user defined header text */
       /* this could be modified to read an external file if required */

   do idx = userheader() to sourceline()
      if (idx//Info_Rate = 0) then call writech('window',star)
      call writeln('guide',sourceline(idx))
   end

       /* finish off the fixed header */

   call writeln('guide','')
   call writeln('guide','@ENDNODE' nl)
   call writeln('guide','@NODE MAIN  "'Guide_title'"' nl)
   call writeln('guide','@{" Infomation " link INFO}' tab 'What is this guide?' nl)

       /* write headers specific for this guide */

   if ~open('header',Temp_header,'r') then call err(Temp_header '(r)')
   do idx = 1 until eof('header')
      if (idx//Info_Rate = 0) then call writech('window',star)
      call writeln('guide',readln('header'))
   end
   call close('header')

       /* write main guide body */

   if ~open('main',Temp_main,'r') then call err(Temp_main '(r)')
   do idx = 1 until eof('main')
      if (idx//Info_Rate = 0) then call writech('window',star)
      call writeln('guide',readln('main'))
   end

   call close('main')    /* drop our lock so we can delete this file */
   call close('guide')   /* the guide now exists */
return

/********************************************************/

err:                          /* tell user of any problems */
   parse arg errText
   call writeln('window',nl nl' ERROR: Cannot open' errText)
   call  readln('window')     /* wait for user input    */
   call   close('window')     /* neat but not actually required */
   exit SIGL                  /* send error line number to the calling process */
return                        /* if we hit this line we have a real problem */

/********************************************************/

userheader:                       /* this function returns its own line number  */
   signal x ; x:                  /* by setting the rexx variable SIGL here     */
return SIGL + 4                   /* but the text starts four lines later       */
exit             /* this stops rexx from falling thru to execute the users text */
/* user information starts on the next line to the EOF and is included as typed */










                      Encoding: quoted-printable

 This transfer standard introduces strange characters into the final guide
          which may make some individual mail items harder to read.

