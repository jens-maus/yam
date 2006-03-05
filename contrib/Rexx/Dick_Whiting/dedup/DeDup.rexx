/******************************************************************************/
/*                                                                            */
/*                            DeDup.rexx                                      */
/*                 Copyright ©1998 by Dick Whiting                            */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*        Find and Delete Duplicate Mail in a YAM 2.x folder                  */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* NOTE: You will need to update the QSORTPROG variable before running.       */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Logic: This script considers 2 mail to be duplicates IFF:                  */
/* 1) From fields are the same, AND                                           */
/* 2) Subject fields are the same, AND                                        */
/* 3) Date fields are the same, AND                                           */
/* 4) Mail sizes in bytes are exactly the same.                               */
/*                                                                            */
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
/*                           24 June 1998                                     */
/*                                                                            */
/******************************************************************************/
/*
$VER: 2.0 Copyright ©1998 by Dick Whiting
$AUTHOR: Dick Whiting
$DESCRIPTION: Find and Delete Duplicate Mail in a YAM 2.x folder
*/

QSORTPROG="SYS:rexxc/QuickSort"

options results

TRUE=1
FALSE=0
missing='.'

/**************************************************************************/
/*                         Initialize Variables                           */
/**************************************************************************/
Call Localize                               /* vars for localizing strings*/
Call Ensurelibs                             /* required libraries         */

/**************************************************************************/
/*                      MAIN LOGIC FOR DEDUP.rexx                         */
/**************************************************************************/

Address YAM 'appbusy 1'
Call GetMinfo                           /* get info for all mail in folder*/
Call FindDups                           /* sort array & identify dups     */
Call DeleteDups                         /* go do the deletes              */
Address YAM 'appnobusy'

exit

/**************************************************************************/
/*                Get From and Subject using YAM facilities               */
/*                                                                        */
/* minfo array: from subject date mailnumber size                         */
/**************************************************************************/
GetMinfo:       

   minfo.=missing
   minfo.0=0

   Address YAM 'GetFolderInfo Path'
   filename=result
   Address YAM 'GetFolderInfo Number'
   anum=result
   Address YAM 'GetFolderInfo Max'
   mailcnt=result
   minfo.0=mailcnt
   do i=0 to mailcnt-1
      mnum=i
      j=i+1
      Address YAM 'Setmail' mnum 
      Address YAM 'Getmailinfo From'
      mfrom=result
      Address YAM 'Getmailinfo Subject'
      msubj=result
      Address YAM 'Getmailinfo Filename'
      mpath=result
      mdate=missing
      msize=missing
      Call GetDate
      mstring=mfrom'|'mdate'|'msize'|'msubj'|'anum'|'right(mnum,4,'0')'|'mpath
      minfo.j=mstring
   end

Return

/**************************************************************************/
/*                Read mail file for Date: line                           */
/**************************************************************************/
GetDate:    

   goodopen=open('IN',mpath,'R')
   datefound=FALSE
   if goodopen then do 
   	finfo=statef(mpath)	
	   parse var finfo type msize rest
      do until eof('IN') | datefound
         linein=readln('IN')
         if linein='' then datefound=TRUE
         if word(linein,1)='Date:' then do
            mdate=subword(linein,2)
            mdate=strip(mdate)
            datefound=TRUE
         end
      end
      result=close('IN')
   end
   else do
      errmsg=_text._badmail
      Call ErrorMsg
      exit
   end

Return

/**************************************************************************/
/*                Sort mail info array and locate duplicates              */
/**************************************************************************/
FindDups:          

   if show('L','rexxtricks.library') then do /* use tricks library        */
      call QSORT(minfo)                      /* sort by name, line number */
   end
   else do                                   /* use QuickSort format      */
      call QSORT(1, minfo.0, minfo)          /* sort by name, line number */
   end 

   dinfo.=missing                            /* array of duplicates       */
   dinfo.0=0  

   oldfrom=missing                           /* set break values          */
   olddate=missing
   oldsize=missing
   oldsubj=missing

   do i=1 to minfo.0
      parse var minfo.i mfrom '|' mdate '|' msize '|' msubj '|' anum '|' mnum '|' mpath
      if mfrom~=oldfrom | mdate~=olddate | msize~=oldsize | msubj~=oldsubj then do
         oldfrom=mfrom
         olddate=mdate   
         oldsize=msize
         oldsubj=msubj
      end
      else do
         j=1+dinfo.0
         dinfo.0=j
         dstring=anum'|'mnum'|'mpath
         dinfo.j=dstring
      end
   end

Return

/**************************************************************************/
/*        Sort Duplicate list, check for list change, do deletes          */
/*    Process in reverse list order to keep from changing list order      */
/**************************************************************************/
DeleteDups:        

   if dinfo.0=0 then do                      /* no duplicates found       */
      errmsg=_text._nodups
      Call ErrorMsg
      exit
   end

   if show('L','rexxtricks.library') then do /* use tricks library        */
      call QSORT(dinfo)                      
   end
   else do                                   /* use QuickSort format      */
      call QSORT(1, dinfo.0, dinfo)          
   end 

   do i=dinfo.0 to 1 by -1
      parse var dinfo.i anum '|' mnum '|' mpath
      Address YAM 'Setfolder' anum
      Address YAM 'Setmail'   mnum
      Address YAM 'Getmailinfo File'
      testpath=result
      if mpath~=testpath then do
         errmsg=_text._chgfolder
         Call ErrorMsg
         exit
      end
      Address YAM 'mailmove 3'
   end

   errmsg=dinfo.0||_text._maildeleted
   Call ErrorMsg
   exit

Return

/**************************************************************************/
/*                                                                        */
/*              Make sure that we have all the libs we need:              */
/*                               RexxSupport                              */
/*                               QuickSort                                */
/*                                                                        */
/**************************************************************************/
EnsureLibs:

   if ~show('L','rexxsupport.library') then do
      addlib('rexxsupport.library',0,-30)
   end

   if ~show('P','QuickSortPort') then do
      address Command "run >nil: "qsortprog
      do i = 1 to 10
         if ~show('P','QuickSortPort') then foo=delay(delaytm)
         else leave i
      end
   end
   if show('P','QuickSortPort') then do
      call addlib('QuickSortPort',-30)
   end
   else do
      errmsg=_text._noqsort
      signal ErrorMsg
   end

Return


/******************************************************************************/
/*  Display ERROR message and EXIT.                                           */
/******************************************************************************/
ErrorMsg:

   Address YAM 'appnobusy'
   Address YAM 'REQUEST "'errmsg'"  "'_text._ok'"'

   exit

Return

/**************************************************************************/
/*       Text, msgs, etc. used                                            */
/**************************************************************************/
Localize:

/*********************************/
/* Miscellaneous info strings    */
/*********************************/

_text._ok="_Ok"                             /* various OK buttons         */

/*********************************/
/* Various error conditions      */
/*********************************/

_text._noqsort="Unable to find QuickSort"
_text._chgfolder="Folder order has changed--quitting this folder"
_text._maildeleted=" mail(s) in folder deleted"
_text._nodups="No Duplicates Found in Folder"
_text._badmail="Unable to open a mailfile--exiting"

Return
