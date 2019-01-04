#!/bin/sh
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
# A shell script to checkout our local subversion repository and then
# pull the current translations from transifex syncing any potential
# changes into the repository.
#

# User definable variables
VERSION="1.1"                               # the version of the tool
GIT=/usr/bin/git                            # path to the git tool
GITROOT="https://github.com/jens-maus/yam.git"        # repository URL
TX=/usr/bin/tx                              # path to transifex cmd-line tool
MODULE=yam-txrobot                          # the main module to checkout
CHECKOUTDIR=/usr/local/amiga/yam-build      # directory where to checkout to
LOCALEDIR=locale

###################################################################
#
# The main stuff starts here
#
echo >&2 "yam-txrobot.sh v${VERSION} - a script to sync transifex translations"
echo >&2 "Copyright (c) 2014-2016 Jens Maus <mail@jens-maus.de>"
echo >&2

# let us do a fresh GIT checkout and see if something
# has been updated or not
echo "checking out GIT repository:"
echo "============================"
if [ ! -e "${CHECKOUTDIR}/${MODULE}/.git" ]; then
  mkdir -p ${CHECKOUTDIR}/${MODULE}
  ${GIT} clone ${GITROOT} ${CHECKOUTDIR}/${MODULE}
fi

# change into our checkout dir
cd ${CHECKOUTDIR}

# now pull all changes
output=`cd ${MODULE}; ${GIT} pull 2>&1`
ret=$?
if [ $ret != 0 ]; then
  echo >&2 "ERROR: git pull failed! aborting."
  echo >&2 "$output"
  exit 2
fi
echo "$output"
echo

# lets pull transifex translations
cd ${MODULE}
echo "pulling newer transifex translations:"
echo "====================================="
output=`${TX} pull`
ret=$?
if [ $ret != 0 ]; then
  echo >&2 "ERROR: tx pull failed! aborting."
  echo >&2 "$output"
  exit 2
fi
echo "$output"
echo

# lets verify if a translations is newer
echo "identify that there is a new transifex translation:"
echo "==================================================="
output=`${GIT} status --short ${LOCALEDIR}`
ret=$?
if [ $ret != 0 ]; then
  echo >&2 "ERROR: git status failed! aborting."
  echo >&2 "$output"
  exit 2
fi
output=`echo "$output" | egrep "^ M .+\.po$"`
ret=$?
if [ $ret != 0 ]; then
  echo "transifex translations are in sync with GIT repository. exiting."
  exit 0
fi
echo "$output"
echo

# now we try to filter out trivial changes
echo "checking for trivial changes to ignore"
echo "======================================"
modifiedfiles=`echo "$output" | awk '{ print $2 }'`
changedfiles=""
for file in ${modifiedfiles}; do
  diff=`${GIT} diff --color=always ${file} | perl -wlne 'print $1 if /^\e\[32m\+\e\[m\e\[32m(.*)\e\[m$/'`

  # ignore whatever we feel is not worth a checkin
  diff=`echo "${diff}" | sed 's/^#.*//'` # comments
  diff=`echo "${diff}" | sed 's/^"Project-Id-Version: .*\\\n".*//'`
  diff=`echo "${diff}" | sed 's/^"Report-Msgid-Bugs-To: .*\\\n".*//'`
  diff=`echo "${diff}" | sed 's/^"POT-Creation-Date: .*\\\n".*//'`
  diff=`echo "${diff}" | sed 's/^"PO-Revision-Date: .*\\\n".*//'`
  diff=`echo "${diff}" | sed 's/^"Last-Translator: .*\\\n".*//'`
  diff=`echo "${diff}" | sed 's/^"Language-Team: .*\\\n".*//'`
  diff=`echo "${diff}" | sed 's/^"MIME-Version: .*\\\n".*//'`
  diff=`echo "${diff}" | sed 's/^"Content-Type: .*\\\n".*//'`
  diff=`echo "${diff}" | sed 's/^"Content-Transfer-Encoding: .*\\\n".*//'`
  diff=`echo "${diff}" | sed 's/^"Language: .*\\\n".*//'`
  diff=`echo "${diff}" | sed 's/^"Plural-Forms: .*\\\n".*//'`
  diff=`echo "${diff}" | tr -d '\n'`

  if [ -n "${diff}" ]; then
    # now we found somewhat relevant changes but have
    # to filter them for things we want to ignore
    echo "relevant changes found in file ${file}"
    changedfiles="${changedfiles} ${file}"
  else
    echo "NO relevant changes found in file ${file}"
  fi
done

# exit if no changed files were identified
if [ ! -n "${changedfiles}" ]; then
  exit 0
fi
echo

# finally commit the changes to our repository, but with the author
# name of transifex
echo "committing modified transifex translations:"
echo "==========================================="
for file in ${changedfiles}; do
  author=`grep "Last-Translator: " ${file} | sed -r 's/\"Last-Translator: (.*)\\\n\"/\\1/'`

  echo "${file}: ${author}"
  output=`${GIT} commit --author="${author}" -m '[tx-robot] updated translation from transifex' ${file} 2>&1`
  ret=$?
  if [ $ret != 0 ]; then
    echo >&2 "ERROR: git commit failed! aborting."
    echo >&2 "$output"
    exit 2
  fi
done

# lets finally push the data
output=$(${GIT} push --quiet 2>&1)
if [ $? != 0 ]; then
  echo >&2 "ERROR: git push failed! aborting."
  echo >&2 "$output"
  exit 2
fi

# exit with no error
exit 0
