/* $VER:RandomSample.rexx  © Kai Nikulainen                                email: kajun@sci.fi 1.0(1-MAY-97)
Plays a random sample whenever mail is received

**  1. Copy the script to YAM:Rexx
**  2. Set YAM settings for 'New mail reporting' to external program
**  3. Enter 'sys:rexxc/rx >nil: yam:rexx/RandomSample.rexx' to the string gadget
**
** If you have any problems, mail me at kajun@sci.fi
**
** Check http://www.sci.fi/~kajun for other scripts
*/

player='c:play16'                       /* Change here the sample player you use */
        
samples=9                               /* Change here the correct number of samples */
file.1='sys:prefs/MasterIhaveMail.wav'  /* Enter each sample on own line */
file.2='sys:prefs/Mail12.wav'           /* You can add as many samples as you want */
file.3='sys:prefs/Mail13.wav'           /* ... well, actually at most 1000 samples */
file.4='sys:prefs/mail14.wav'
file.5='sys:prefs/mail10.wav'
file.6='sys:prefs/mail9.wav'
file.7='sys:prefs/mail8.wav'
file.8='sys:prefs/mail7.wav'
file.9='sys:prefs/mail5.wav'

samplenum=random(1,samples,time(s))

address command player file.samplenum
