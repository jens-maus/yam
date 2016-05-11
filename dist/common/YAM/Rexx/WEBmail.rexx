/* WEBmail.rexx - Send mail from your web browser using YAM               */
/* $VER: WEBmail.rexx 2.3 (18.01.99) © 1999 by Marcel Beck <mbeck@yam.ch> */
/*                                                                        */
/* Usage from VOYAGER:                                                    */
/*  Mailto app = sys:rexxc/rx yam:rexx/webmail %t %s                      */
/*                                                                        */
/* Usage from IBROWSE:                                                    */
/*  External mailto command = sys:rexxc/rx yam:rexx/webmail %h %s         */
/*                                                                        */
/* Usage from AWEB:                                                       */
/*  Mailto command = sys:rexxc/rx                                         */
/*  Mailto arguments = yam:rexx/webmail %e                                */

PARSE ARG email' 'subject
IF ~SHOW('Ports','YAM') THEN
  ADDRESS COMMAND 'Run <>NIL: YAM:YAM NOCHECK MAILTO 'email
ELSE DO
  ADDRESS YAM
  Show
  MailWrite
  WriteTo email
  'WriteSubject "'subject'"'
END
