/*AirMailPro3 -> YAM Converter by Luca Nonato
$VER:[1m0.90b [0m(07.06.99) by [1m[33mLuca Nonato[0m
*/

options results
signal on syntax
signal on ioerr
signal on error

if( ~show( 'l', "rexxsupport.library" ) ) then
do
   if( ~addlib( "rexxsupport.library", 0, -30, 0 ) )then
   do
      say "Could not open rexxsupport.library"
      exit 10
   end
end

ver='0.90b'
AMPDefault="Internet:AirMailPro3/InBox/"
YAMDefault="YAM:"

parse arg AMP3 YamDir RunMode .

say " "
say '[0m[33m[1mAirMailPro3 -> YAM Converter '||ver||' [0mCopyright © 1999 by [1mLuca Nonato[0m'
say " "

if strip(AMP3)='?' then do
        say " "
        say "[1m[4mUsage:[0m[33m[1m AMP3-2-YAM [0m[AirMailPro 3 Directory] [YAM Directory] [/SIMULATE]"
        say " "
        say "When no parameter is given, i'll assume:"
        say "[1mAirMailPro 3 Directory[0m = " || AMPDefault
        say "[1mYAM Directory[0m = " || YAMDefault
        say " "
        say "Use /SIMULATE to simulate (You guessed?) the operation and see on screen"
        say "the shell-commands"
        say " "
        exit
end

IF ~SHOW('Ports', 'YAM') THEN DO
    ADDRESS 'YAM'
    Say 'Yam not active? :-)'
    EXIT 0
END

if ~(RunMode = "") & ~(RunMode = "/SIMULATE") then say "Parameter [1m" || RunMode || "[0m incorrect. Ignored"

if (AMP3 = "/SIMULATE") && (YamDir = "/SIMULATE") && (RunMode = "/SIMULATE") then RunMode = "FakeIt"
   else RunMode = "MakeItSo"

If AMP3 = "/SIMULATE" then AMP3 = ""
If YAMDir = "/SIMULATE" then YAMDir = ""
if AMP3 = "" then AMP3=AMPDefault
if YaMDir = "" then YamDir=YAMDefault

if ~Exists(AMP3) then do
   Say "[1mFATAL ERROR!!![0m AirMailPro dir (" || AMP3 || ") not found!"
   exit
end

if ~Exists(YAMDir) then do
   Say "[1mFATAL ERROR!!![0m YAM dir (" || YAMDir || ") not found! Exiting..."
   exit
end

FileMode = "Input"

do until (FileMode ~= "C") && (FileMode ~= "M")
   Say " "
   Say "Do you like to (C)opy or (M)ove the messages?"
   Pull FileMode
end

if FileMode = "C" then FileMode = "Copy"
                  else FileMode = "Move"


Say "Please Wait... I'm building the filelist. This should be pretty quick, though... :)"
TempFile = "T:TMP.CPD"

CmdString = "List" AMP3 "LFORMAT ""%n"" >" TempFile
Say "Shell Command: " || CmdString
ADDRESS COMMAND CmdString

if RunMode = "FakeIt" then do
   Say "Not in [1msimulation[0m mode, now i'll do:"
   Say " "
end

call open FileList,'T:TMP.CPD',R
do until eof(FileList)
        Line1 = readln(FileList)
        if Line1 = "" then iterate
        Say "Processing " || Line1
        ADDRESS YAM 'NEWMAILFILE VAR Stringa Incoming'
        CmdLine = FileMode || " " || AMP3 || Line1 || " " || YamDir || Stringa
        if RunMode = "FakeIt" then Say CmdLine
                              else ADDRESS COMMAND CmdLine
end

if RunMode = "MakeItSo" then do
   ADDRESS YAM 'SETFOLDER incoming'
   ADDRESS YAM 'MAILUPDATE'
end

CmdString = "Delete " TempFile
ADDRESS COMMAND CmdString
