#!/bin/sh
#
# YAM - Yet Another Mailer
# Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
# Copyright (C) 2000-2014 YAM Open Source Team
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
# $Id$
# $URL$
#

# User definable variables
VERSION="1.0"                               # the version of the tool
SVN=/usr/bin/svn                            # path to the subversion tool
SVNROOT="file:///home/svn/yam/trunk"        # repository URL
TX=/usr/bin/tx                              # path to transifex cmd-line tool
MODULE=yam-txrobot                          # the main module to checkout
CHECKOUTDIR=/usr/local/amiga/yam-build      # directory where to checkout to

###################################################################
#
# The main stuff starts here
#
echo >&2 "yam-txrobot.sh v${VERSION} - a script to sync transifex translations"
echo >&2 "Copyright (c) 2014 Jens Maus <mail@jens-maus.de>"
echo >&2

# let us do a fresh SVN checkout and see if something
# has been updated or not
echo "checking out SVN repository:"
echo "============================"
cd $CHECKOUTDIR
output=`${SVN} checkout --config-option config:miscellany:use-commit-times=yes ${SVNROOT} ${MODULE}`
ret=$?
if [ $ret != 0 ]; then
   echo >&2 "ERROR: svn checkout failed! aborting."
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
output=`${SVN} status ${LOCALEDIR}` 
ret=$?
if [ $ret != 0 ]; then
   echo >&2 "ERROR: svn status failed! aborting."
   echo >&2 "$output"
   exit 2
fi
echo "$output" | egrep "^M .+\.po$" >/dev/null
ret=$?
if [ $ret != 0 ]; then
   echo "transifex translations are in sync with SVN repository. exiting."
   exit 0
fi
echo "$output"
echo

echo "committing modified transifex translations:"
echo "==========================================="
files=`echo "$output" | awk '{ print $2 }'`
for file in ${files}; do
   user=`grep "Last-Translator:" ${file} | awk '{ gsub(/\\\\n"/, ""); print tolower($2) }'`

   echo "${user}: ${file}"
   output=`${SVN} commit -m '[tx-robot] updated translation from transifex' --non-interactive --username ${user} ${file}`
   ret=$?
   if [ $ret != 0 ]; then
      echo >&2 "ERROR: svn commit failed! aborting."
      echo >&2 "$output"
      exit 2
   fi
done

# exit with no error
exit 0
