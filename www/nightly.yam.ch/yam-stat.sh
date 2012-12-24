#!/bin/sh
#
# A shell script to directly download the latest SVN sources of
# the YAM project and builds different snapshots automatically
# each night
#
# Copyright (c) 2004-2006 Jens Langner <Jens.Langner@light-speed.de>
#

# User definable variables
SVN=/usr/bin/svn                     # path to the subversion tool
JAVA=java 
SVNROOT="file:///home/svn/yam/"
MODULE=yam                         # the main module to checkout
CHECKOUTDIR=/usr/local/amiga/yam-build   # directory where to checkout to 
WEBDIR=/var/www/www.yam.ch/cowiki/htdocs/stats     # path to where we put the builds
SVNSTAT=/usr/local/statsvn/statsvn.jar
NICE=nice                            # path to nice tool

# lets add additional pathes for our script
MODULEPATH=${CHECKOUTDIR}/${MODULE}

###################################################################
#
# The main stuff starts here
#

printf "generating SVN logfile... "
${NICE} ${SVN} log -v --xml ${SVNROOT} >${CHECKOUTDIR}/svnlog.log
printf "ok!\n"

printf "generating svn statistics pages... "
${NICE} ${JAVA} -jar ${SVNSTAT} -title "YAM" \
                                -tags '2.*' \
                                -tags-dir '/releases/' \
                                -trac 'http://yam.ch/' \
                                -include '**/*.c:**/*.h:**/*.l:**/*.sd:**/*Makefile*' \
                                -exclude '**/release/**:**/www/**:**/icons/**:**/supportfiles/**:**/doc/**:**/include/**:**/includes/**:**/lib/**:**/ChangeLog:**/locale/**:**/themes/**:**/*.depend:**/*.dep' \
                                -notes ${CHECKOUTDIR}/stat-notes.txt \
                                -output-dir ${WEBDIR} \
                                ${CHECKOUTDIR}/svnlog.log ${MODULEPATH}
printf "ok!\n"
