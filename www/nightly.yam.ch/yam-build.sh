#!/bin/sh
#
# YAM - Yet Another Mailer
# Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
# Copyright (C) 2000-2015 YAM Open Source Team
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
# YAM Official Support Site :  http://yam.ch/
#
# A shell script to directly download the latest SVN sources of
# the YAM project and builds different snapshots automatically
# each night
#
# $Id$
# $URL$
#

# User definable variables
VERSION="1.1"                        # the version of the tool
SVN=/usr/bin/svn                     # path to the subversion tool
SVNROOT="file:///home/svn/yam/trunk"
MODULE=yam                           # the main module to checkout
CHECKOUTDIR=/usr/local/amiga/yam-build   # directory where to checkout to
MAKE="make -j1"                      # path to GNU make tool
LHA=lha                              # path to lha archive tool
WEBDIR=/var/www/www.yam.ch/nightly   # path to where we put the builds
MD5SUM=md5sum                        # path to md5sum tool
NICE=nice                            # path to nice tool
UPDCHKPATH="/var/www/www.yam.ch/update/updates/nightly" # path to the update check directory

# lets add additional pathes for our script
export PATH="/usr/local/amiga/gg/bin:/usr/local/amiga/bin:$PATH"
MODULEPATH=${CHECKOUTDIR}/${MODULE}
BUILDID=`date +%Y%m%d`
BUILDVER="2.10"
BUILDV=`echo ${BUILDVER} | tr -d "."`
DEVDIR=$CHECKOUTDIR/`date +%F-dev`

###################################################################
#
# support functions

# a function to output the general help/usage page
displayUsage()
{
  echo >&2
  echo >&2 "Usage: $0 <options>"
  echo >&2 "Options:"
  echo >&2 "  -q : be quiet while processing"
  echo >&2 "  -f : force a rebuild"
  echo >&2 "  -h : display this help page"
  exit 2
}

# function to open/create a logfile and redirect all output
openLogFile()
{
   OUTPUT_PIPE="/tmp/yam-build_$$.pipe"
   OUTPUT_LOG="$1"

   if [ ! -e ${OUTPUT_PIPE} ]; then
      mkfifo ${OUTPUT_PIPE}
   fi

   if [ -e ${OUTPUT_LOG} ]; then
      rm ${OUTPUT_LOG}
   fi

   # add a trap handler which makes sure the logfile and pipe will be
   # properly closed/deleted
   trap closeLogFile 0 1 2 3 6 15

   # start the redirection
   exec 3>&1 4>&2
   tee ${OUTPUT_LOG} <${OUTPUT_PIPE} >&3 &
   LOG_PID=$!
   exec >${OUTPUT_PIPE} 2>&1

   # now the redirection is finished
   echo "Redirecting output to logfile: ${OUTPUT_LOG}"
   echo "Output generated on `date`"
   echo
}

# function to close the logfile.
closeLogFile()
{
   echo "Closing logfile ${OUTPUT_LOG}"

   exec 1>&3 3>&- 2>&4 4>&-
   wait $LOG_PID
   rm ${OUTPUT_PIPE}

   trap "" 0 1 2 3 6 15
}

# function to compile a RELEASE version for
# a certain target
compile_release()
{
  TARGET=$1       # e.g. "AmigaOS4"
  TARGETEXT=$2    # e.g. "os4"

  cd $MODULEPATH
  echo "${TARGET}: Compiling RELEASE version"
  echo "================================================================="
  set -x
  $NICE -n 19 $MAKE OS=${TARGETEXT} DEBUG= BUILDID="$BUILDID" distclean all
  ret=$?
  set +x
  echo "-----------------------------------------------------------------"
  if [ $ret = 0 ] && [ -e "src/YAM.${TARGETEXT}" ]; then
    echo "archiving:"
    set -x
    cp src/YAM.${TARGETEXT} $DEVDIR/YAM
    cp $MODULEPATH/icons/${TARGETEXT}/YAM.info $DEVDIR/
    cd $DEVDIR
    $LHA ao5 YAM${BUILDV}dev-${TARGET}.lha YAM YAM.info ChangeLog README.txt catalogs resources >/dev/null 2>&1
    $MD5SUM YAM${BUILDV}dev-${TARGET}.lha >YAM${BUILDV}dev-${TARGET}.lha.md5
    rm YAM
    set +x
    echo "done."
  else
    echo "error during compile"
  fi
  echo "================================================================="
}

# function to compile a DEBUG version for
# a certain target
compile_debug()
{
  TARGET=$1       # e.g. "AmigaOS4"
  TARGETEXT=$2    # e.g. "os4"

  cd $MODULEPATH
  echo "${TARGET}: Compiling DEBUG version"
  echo "================================================================="
  set -x
  $NICE -n 19 $MAKE OS=${TARGETEXT} DEBUG="-DDEBUG -g -O0 -fno-omit-frame-pointer" BUILDID="$BUILDID" distclean all
  ret=$?
  set +x
  echo "-----------------------------------------------------------------"
  if [ $ret = 0 ] && [ -e "src/YAM.${TARGETEXT}.debug" ]; then
    echo "archiving:"
    set -x
    cp src/YAM.${TARGETEXT}.debug $DEVDIR/YAM.debug
    cp $MODULEPATH/icons/${TARGETEXT}/YAM.info $DEVDIR/YAM.debug.info
    cd $DEVDIR
    $LHA ao5 YAM${BUILDV}dev-${TARGET}-debug.lha YAM.debug YAM.debug.info ChangeLog README.txt catalogs resources >/dev/null 2>&1
    $MD5SUM YAM${BUILDV}dev-${TARGET}-debug.lha >YAM${BUILDV}dev-${TARGET}-debug.lha.md5
    rm YAM.debug
    set +x
    echo "done."
  else
    echo "error during compile!"
  fi
  echo "================================================================="
}

# to generate all catalogs
create_catalogs()
{
  echo "Creating catalog files:"
  echo "================================================================="
  set -x
  mkdir $DEVDIR/catalogs
  cd $MODULEPATH/src
  rm ../locale/*.catalog
  $MAKE catalogs
  cd $MODULEPATH/locale
  for cat in *.catalog; do
    mkdir $DEVDIR/catalogs/${cat%.catalog}
    cp $cat $DEVDIR/catalogs/${cat%.catalog}/YAM.catalog
  done
  set +x
  echo "================================================================="
  echo "done."
}

###################################################################
#
# The main stuff starts here
#
echo >&2 "yam-build.sh v${VERSION} - a script to build the nightly for YAM"
echo >&2 "Copyright (c) 2004-2014 Jens Maus <mail@jens-maus.de>"
echo >&2

# define the variables we know
force=
quiet=

# parse for command-line options
while getopts "qfh" opt
do
  case "$opt" in
    q)  quiet="yes";;
    f)  force="force";;
    h)  displayUsage;;
    \?) # unknown flag
        displayUsage;;
  esac
done
shift `expr $OPTIND - 1`

# start logging everything
openLogFile `pwd`/yam-build.log

# if the user decided to be quiet we redirect the standard
# output to null
if [ "$quiet" = "yes" ]; then
   exec 1>&-
fi

# let us automatically delete all build directories which are older than 60 days
echo "cleaning up nightly builds older than 60 days:"
echo "============================================="
find $WEBDIR/ -maxdepth 1 -type d -daystart -mtime +60 -print -exec rm -rf {} \;
echo "============================================="

# let us do a fresh SVN checkout and see if something
# has been updated or not
echo "checking out SVN repository:"
echo "============================"
cd $CHECKOUTDIR
output=`${SVN} co ${SVNROOT} ${MODULE}`
ret=$?
if [ $ret != 0 ]; then
   echo "error during checkout! aborting."
   echo $output
   exit 2
fi
echo "$output" | egrep "^[UPAG] .+\.[chl][d]*$" >/dev/null
ret=$?
echo "$output"
echo "============================"
if [ "$force" != "force" ]; then
   if [ $ret != 0 ]; then
      printf "no relevant changes found. checking last build date..."
      last_build=`cat .last_build`
      today=`expr \`date +%s\` / 86400 - 2922 - $last_build`
      printf "$today days passed..."
      if [ $today -gt 29 ]; then
         printf "rebuilding.\n"
      else
         printf "no rebuild required.\n"
         exit 0
      fi
   fi
else
   echo "forcing rebuild."
fi

# create a new dev directory
printf "Generating new dev directory ["
rm -rf $CHECKOUTDIR/*-dev
printf "$DEVDIR]..."
mkdir $DEVDIR
printf "done.\n"

# if we end up here then something has changed since the last checkout
# so lets build everything right from the start
cp $WEBDIR/README.txt $DEVDIR/
cp $MODULEPATH/ChangeLog $DEVDIR/

# copy the resources from the respository to a local copy
cp -a $MODULEPATH/resources $DEVDIR/ >/dev/null 2>&1

# delete Subversion's database files
find $DEVDIR/resources/ -name ".svn" -exec rm -rf {} \; >/dev/null 2>&1

# let us generate all catalogs first
create_catalogs

# AmigaOS4 target compile
compile_release AmigaOS4 os4
compile_debug AmigaOS4 os4

# AmigaOS3/m68k target compile
compile_release AmigaOS3 os3
compile_debug AmigaOS3 os3

# MorphOS/PPC target compile
compile_release MorphOS mos
compile_debug MorphOS mos

# AROS/i386 target compile
compile_release AROSi386 aros-i386
compile_debug AROSi386 aros-i386

# AROS/ppc target compile
compile_release AROSppc aros-ppc
compile_debug AROSppc aros-ppc

# AROS/x86_64 target compile
compile_release AROSx86_64 aros-x86_64
compile_debug AROSx86_64 aros-x86_64

# AROS/ARM target compile
compile_release AROSarm aros-arm
compile_debug AROSarm aros-arm

# then delete the temporary stuff again
rm $DEVDIR/ChangeLog $DEVDIR/README.txt
rm -rf $DEVDIR/catalogs
rm -rf $DEVDIR/resources
rm $DEVDIR/YAM.info
rm $DEVDIR/YAM.debug.info

# then finally put up the archives on the webserver.
cd $MODULEPATH
echo "putting stuff on webserver:"
set -x
head -n 100 ChangeLog >$DEVDIR/ChangeLog-`date +%F`
printf "\n\nTHIS IS JUST A 100 LINE STRIPPED VERSION OF THE CHANGELOG\n" >>$DEVDIR/ChangeLog-`date +%F`
rm -rf $WEBDIR/`basename $DEVDIR`
mv -f $DEVDIR $WEBDIR
cd $WEBDIR
rm -f latest-dev
ln -sf `basename $DEVDIR` latest-dev
set +x

# now we can also update the updatecheck META file accordingly.
echo "creating new updatecheck file:"
printf "VERSION: ${BUILDVER}-dev\n" >${UPDCHKPATH}/${BUILDVER}
printf "BUILDID: ${BUILDID}\n" >>${UPDCHKPATH}/${BUILDVER}
printf "BUILDDATE: `date +%d.%m.%Y`\n" >>${UPDCHKPATH}/${BUILDVER}
printf "URL: m68k-amigaos http://nightly.yam.ch/`date +%F-dev`/YAM${BUILDV}dev-AmigaOS3.lha\n" >>${UPDCHKPATH}/${BUILDVER}
printf "URL: ppc-amigaos http://nightly.yam.ch/`date +%F-dev`/YAM${BUILDV}dev-AmigaOS4.lha\n" >>${UPDCHKPATH}/${BUILDVER}
printf "URL: ppc-morphos http://nightly.yam.ch/`date +%F-dev`/YAM${BUILDV}dev-MorphOS.lha\n" >>${UPDCHKPATH}/${BUILDVER}
printf "URL: i386-aros http://nightly.yam.ch/`date +%F-dev`/YAM${BUILDV}dev-AROSi386.lha\n" >>${UPDCHKPATH}/${BUILDVER}
printf "URL: ppc-aros http://nightly.yam.ch/`date +%F-dev`/YAM${BUILDV}dev-AROSppc.lha\n" >>${UPDCHKPATH}/${BUILDVER}
printf "URL: x86_64-aros http://nightly.yam.ch/`date +%F-dev`/YAM${BUILDV}dev-AROSx86_64.lha\n" >>${UPDCHKPATH}/${BUILDVER}
printf "URL: arm-aros http://nightly.yam.ch/`date +%F-dev`/YAM${BUILDV}dev-AROSarm.lha\n" >>${UPDCHKPATH}/${BUILDVER}
printf "CHANGES:\n"  >>${UPDCHKPATH}/${BUILDVER}
head -n 100 ${MODULEPATH}/ChangeLog >>${UPDCHKPATH}/${BUILDVER}
printf "\n\nTHIS IS JUST A 100 LINE STRIPPED VERSION OF THE CHANGELOG\n" >>${UPDCHKPATH}/${BUILDVER}

# we write out the number of days to our last-build file
# so that at least every 31 days a new YAM is automatically build even if no
# changes had been applied
cd $CHECKOUTDIR
expr `date +%s` / 86400 - 2922 >.last_build
echo "done."

# close the logfile
closeLogFile

# move logfile to WEBDIR
cp ${OUTPUT_LOG} $WEBDIR/`basename $DEVDIR`/

# exit with no error
exit 0
