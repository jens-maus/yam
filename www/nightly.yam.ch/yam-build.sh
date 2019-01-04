#!/bin/bash
#
# YAM - Yet Another Mailer
# Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
# Copyright (C) 2000-2019 YAM Open Source Team
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
# A shell script to directly download the latest GIT sources of
# the YAM project and builds different snapshots automatically
# each night
#

# User definable variables
VERSION="1.5"                        # the version of the tool
GIT=/usr/bin/git                     # path to the git tool
GITROOT="https://github.com/jens-maus/yam.git"
MODULE=yam                           # the main module to checkout
CHECKOUTDIR=/usr/local/amiga/yam-build   # directory where to checkout to
MAKE="make -j1"                      # path to GNU make tool
LHA=lha                              # path to lha archive tool
WEBDIR=/var/www/www.yam.ch/nightly   # path to where we put the builds
MD5SUM=md5sum                        # path to md5sum tool
NICE=nice                            # path to nice tool
UPDCHKPATH="/var/www/www.yam.ch/update/updates/nightly" # path to the update check directory

# lets add additional pathes for our script
export PATH="/opt/ppc-morphos/bin:/opt/m68k-amigaos/bin:/opt/ppc-amigaos/bin:/usr/local/amiga/bin:/usr/local/amiga/gg/bin:$PATH"
MODULEPATH=${CHECKOUTDIR}/${MODULE}
DEVDIR=${CHECKOUTDIR}/$(date +%F-dev)

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

   if [[ ! -e ${OUTPUT_PIPE} ]]; then
      mkfifo ${OUTPUT_PIPE}
   fi

   if [[ -e ${OUTPUT_LOG} ]]; then
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

# function to compile and create the nightly build archives
create_nightlies()
{
  cd $MODULEPATH
  echo "Creating nightly build archives"
  echo "================================================================="
  set -x
  $NICE -n 19 $MAKE nightly LAST_BUILD=${last_build}
  ret=$?
  set +x
  echo "-----------------------------------------------------------------"
  if [[ ${ret} -eq 0 ]]; then
    echo "Moving nightly build archives:"
    set -x
    mv -v *.lha *.lha.md5 ${DEVDIR}/
    set +x
    echo "done."
  else
    echo "ERROR: couldn't create all nightly build archives."
  fi
  echo "================================================================="
}

###################################################################
#
# The main stuff starts here
#
echo >&2 "yam-build.sh v${VERSION} - a script to create the nightly builds for YAM"
echo >&2 "Copyright (c) 2004-2017 Jens Maus <mail@jens-maus.de>"
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
openLogFile $(pwd)/yam-build.log

# if the user decided to be quiet we redirect the standard
# output to null
if [[ ${quiet} == yes ]]; then
  exec 1>&-
fi

# let us automatically delete all build directories which are older than 60 days
echo "cleaning up nightly builds older than 60 days:"
echo "============================================="
find $WEBDIR/ -maxdepth 1 -type d -daystart -mtime +60 -print -exec rm -rf {} \;
echo "============================================="

# let us do a fresh GIT checkout and see if something
# has been updated or not
echo
echo "updating GIT repository:"
echo "======================="
if [[ ! -e ${CHECKOUTDIR}/${MODULE}/.git ]]; then
  mkdir -p ${CHECKOUTDIR}/${MODULE}
  ${GIT} clone ${GITROOT} ${CHECKOUTDIR}/${MODULE}
  force="force"
fi

# change into our checkout dir
cd $CHECKOUTDIR

# get the time of the last build
last_build=0
if [[ -e ".last_build" ]]; then
  last_build=$(cat .last_build)
fi

# undo all possible local changes to the local working copy so that
# we won't get any conflicts.
(cd ${MODULE}; ${GIT} reset --hard; ${GIT} clean -d -x -f)

# now pull changes
output=$(cd ${MODULE}; ${GIT} pull 2>&1)
if [[ ${force} != "force" ]]; then
  ret=1
  update_output=$(echo "${output}" | egrep "^ .+\.[chl][d]*")
  echo "${output}" | egrep "^ .+\.[chl][d]*" >/dev/null
  ret=$?
  if [[ ${ret} -eq 0 ]]; then
    echo "${update_output}"
  fi
  if [[ ${ret} -ne 0 ]]; then
    echo "${output}"
    echo "============================"
    echo -n "no relevant changes found. checking last build date..."
    today=`expr \( \`date +%s\` - ${last_build} \) / 86400`
    echo -n "${today} days passed..."
    if [[ ${today} -gt 28 ]]; then
      echo "rebuilding."
    else
      echo "no rebuild required."
      exit 0
    fi
  else
    echo "============================"
    echo "relevant changes found, running build."
  fi
else
  echo "forcing rebuild."
fi

# create a new dev directory
echo -n "Generating new dev directory ["
rm -rf ${CHECKOUTDIR}/*-dev
echo -n "${DEVDIR}]..."
mkdir -p ${DEVDIR}
echo "done."

# create the nightly builds
create_nightlies

# now we generate an automatic ChangeLog from the git history
cd ${CHECKOUTDIR}
if [ ${last_build} -eq 0 ]; then
  since="-10" # last 10 commits
else
  since="--since=${last_build}"
fi
changelog=$(cd ${MODULE}; git log ${since} --pretty=tformat:'%ad %an <%ae>%n%n  %B' --date=short)
if [ -z "${changelog}" ]; then
  changelog="This is only a rebuild of YAM with no functionaly changes but updated expiration date."
fi

# then finally put up the archives on the webserver.
cd ${MODULEPATH}
echo "putting stuff on webserver:"
set -x
echo "${changelog}" | head -c 8K >${DEVDIR}/ChangeLog-$(date +%F)
rm -rf ${WEBDIR}/$(basename ${DEVDIR})
mv -f ${DEVDIR} ${WEBDIR}
cd ${WEBDIR}
rm -f "latest-dev"
ln -sf $(basename ${DEVDIR}) "latest-dev"
set +x

# now we can also update the updatecheck META file accordingly.
BUILDID=$(date +%Y%m%d)
BUILDVER=$(grep "#define __YAM_VERSION" ${MODULEPATH}/src/YAM_global.c | cut -d "\"" -f2)
BUILDV=$(echo ${BUILDVER} | tr -d ".")
echo "creating new updatecheck file:"
printf "VERSION: ${BUILDVER}-dev\n" >${UPDCHKPATH}/${BUILDVER}
printf "BUILDID: ${BUILDID}\n" >>${UPDCHKPATH}/${BUILDVER}
printf "BUILDDATE: $(date +%d.%m.%Y)\n" >>${UPDCHKPATH}/${BUILDVER}
printf "URL: m68k-amigaos http://nightly.yam.ch/$(date +%F-dev)/YAM${BUILDV}dev-${BUILDID}-AmigaOS3.lha\n" >>${UPDCHKPATH}/${BUILDVER}
printf "URL: ppc-amigaos http://nightly.yam.ch/$(date +%F-dev)/YAM${BUILDV}dev-${BUILDID}-AmigaOS4.lha\n" >>${UPDCHKPATH}/${BUILDVER}
printf "URL: ppc-morphos http://nightly.yam.ch/$(date +%F-dev)/YAM${BUILDV}dev-${BUILDID}-MorphOS.lha\n" >>${UPDCHKPATH}/${BUILDVER}
printf "URL: i386-aros http://nightly.yam.ch/$(date +%F-dev)/YAM${BUILDV}dev-${BUILDID}-AROSi386.lha\n" >>${UPDCHKPATH}/${BUILDVER}
printf "URL: ppc-aros http://nightly.yam.ch/$(date +%F-dev)/YAM${BUILDV}dev-${BUILDID}-AROSppc.lha\n" >>${UPDCHKPATH}/${BUILDVER}
printf "URL: x86_64-aros http://nightly.yam.ch/$(date +%F-dev)/YAM${BUILDV}dev-${BUILDID}-AROSx86_64.lha\n" >>${UPDCHKPATH}/${BUILDVER}
printf "URL: arm-aros http://nightly.yam.ch/$(date +%F-dev)/YAM${BUILDV}dev-${BUILDID}-AROSarm.lha\n" >>${UPDCHKPATH}/${BUILDVER}
printf "CHANGES:\n"  >>${UPDCHKPATH}/${BUILDVER}
echo "${changelog}" | head -c 8K >>${UPDCHKPATH}/${BUILDVER}

# we write out the number of days to our last-build file
# so that at least every 31 days a new YAM is automatically build even if no
# changes had been applied
cd $CHECKOUTDIR
date +%s >.last_build
echo "done."

# close the logfile
closeLogFile

# move logfile to WEBDIR
cp ${OUTPUT_LOG} ${WEBDIR}/$(basename ${DEVDIR})/

# exit with no error
exit 0
