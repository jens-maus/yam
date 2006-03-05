/**************************************************************************/
/*                              WEBmail.rexx                              */
/*                                                                        */
/* Send mail from your web browser using YAM                              */
/* (c) by Marcel Beck <mbeck@access.ch>                                   */
/*                                                                        */
/* Updated for IBROWSE 1.10 by Dick Whiting 24March1997                   */
/* Updated for Yam2.0p4     by Dick Whiting 29April1998                   */
/*                                                                        */
/* Version 2.4:             by Dick Whiting 09Sept1998                    */
/* Added handling for other forms of mailto: ala Alexander Niven-Jenkins  */
/* Added variable user for starting Yam with a default user. If this is   */
/*  not needed in your configuration set user=''                          */
/* Added variable for default subject when ?subject not present           */
/*  for NO default subject set default=''                                 */
/*                                                                        */
/* Notes: In the following usage information:                             */
/*                                                                        */
/* replace "sys:rexxc/rx" with where your rx command is if it is          */
/* not in its normal location.                                            */
/*                                                                        */
/* replace "Yam:rexx/webmail.rexx" with its ACTUAL name and location      */
/*                                                                        */
/* Usage from IBROWSE 1.10, 1.20:                                         */
/*  External mailto command = sys:rexxc/rx Yam:rexx/webmail.rexx "%h"|"%s"*/
/*                                                                        */
/* Usage from VOYAGER (2.70)                                              */
/*  Mailto app = sys:rexxc/rx Yam:rexx/webmail.rexx "%h"                  */
/*                                                                        */
/* Usage from VOYAGER (2.95) thanks to Rens van Es                        */
/*  Mailto app = sys:rexxc/rx Yam:rexx/webmail.rexx "%t"|"%s"             */
/*                                                                        */
/* Usage from AWEB (3.1) thanks to Mike Leavitt                           */
/*  Mailto command = sys:rexxc/rx                                         */
/*  Mailto arguments = Yam:rexx/webmail.rexx "%e"|"%s"                    */
/*                                                                        */
/**************************************************************************/
/*   
$VER: WEBmail.rexx 2.4 (09Sep98)  
*/  

/**************************************************************************/
/* Edit the following two variables as desired or required                */
/**************************************************************************/
user='USER _Dick'                 /* Start Yam with this default USER     */
default='"Message from your URL"' /* Your default subject--keep ALL quotes*/


PARSE ARG email '|' subject    

IF strip(subject,'B','"')=''|pos('(No_subject)',subject)>0 THEN subject=default

IF ~SHOW('Ports','YAM') THEN    
  ADDRESS COMMAND 'Run >NIL: YAM:YAM 'user' NOCHECK MAILTO 'email' SUBJECT 'subject    
ELSE DO
  ADDRESS YAM
  'Show'
  'MailWrite'
  'WriteTo' email
  'WriteSubject' subject
END

