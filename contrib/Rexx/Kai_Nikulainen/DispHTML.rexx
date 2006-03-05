/*$VER:DispHTML.rexx  © Kai Nikulainen                                 email: kajun@sci.fi 1.0 (1-Aug-97)
HTML viewer for your browser

Displays a HTML attachment with your browser.

Set MIME viewer for text/HTML to sys:rexxc/rx yam:rexx/dispHTML.rexx %s
and copy this script to YAM:rexx.

Check also http://www.sci.fi/~kajun for lots of other scripts
*/
options results
parse arg file

browser='run <>nil: Work:IBrowse/IBrowse'    /* Change this to suit your system */

address command 'copy >nil:' file 't:letter.html'
html='file://localhost/t:letter.html'

no_browser=1

lst=show('P')

if pos('MINDWALKER',lst)>0 then do
  address 'MINDWALKER' 'OpenURL '|| html
  no_browser=0
  end

if pos('VOYAGER',lst)>0 then do
  address 'VOYAGER' 'OpenURL '|| html
  no_browser=0
  end

if pos('IBROWSE',lst)>0 then do
  address 'IBROWSE' 'GotoURL ' || html
  no_browser=0
  end

if pos('AWEB',lst)>0 then do
  address value substr(lst,pos('AWEB.',lst),6)          
  'Open ' || html
  no_browser=0
  end

if no_browser then address command browser html

exit
