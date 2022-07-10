#!/bin/bash
############################################################################
#
# YAM - Yet Another Mailer
# Copyright (C) 1995-2000 Marcel Beck
# Copyright (C) 2000-2022 YAM Open Source Team
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
# YAM Official Support Site :  http://www.yam.ch
# YAM OpenSource project    :  http://sourceforge.net/projects/yamos/
#
############################################################################

# YAM archive build script
# invoke this script as "./mkarchive.sh [release|nightly] [os4|os3|mos|aros-i386|aros-ppc|aros-x86_64|aros-arm]"
# to build lha archives for AmigaOS4.x, etc.

# check the first option
case $1 in
  release)     archtype="release"; compileflags="DEVFLAGS=";;
  nightly)     archtype="nightly"; compileflags="BUILDID=$(date +%Y%m%d)";;
  *)           echo "ERROR: '$1' option (archive type) unknown."; exit 2;;
esac

# check second option
case $2 in
  os3)         yamsys="os3"; yamplatform="os3"; yamarchive="AmigaOS3";;
  os4)         yamsys="os4"; yamplatform="os4"; yamarchive="AmigaOS4";;
  mos)         yamsys="mos"; yamplatform="mos"; yamarchive="MorphOS";;
  aros-i386)   yamsys="aros-i386"; yamplatform="aros"; yamarchive="AROSi386";;
  aros-ppc)    yamsys="aros-ppc"; yamplatform="aros"; yamarchive="AROSppc";;
  aros-x86_64) yamsys="aros-x86_64"; yamplatform="aros"; yamarchive="AROSx86_64";;
  aros-arm)    yamsys="aros-arm"; yamplatform="aros"; yamarchive="AROSarm";;
  *)           echo "ERROR: '$2' option (operating system) unknown."; exit 2;;
esac

echo "  MK ${yamsys} ${archtype}"

# get the current version number
yamver=$(grep "#define __YAM_VERSION" src/YAM_global.c | cut -d "\"" -f2)

if [[ ${archtype} == "nightly" ]]; then
  # nightly build
  yamarcver="$(echo ${yamver} | tr -d ".")dev-$(date +%Y%m%d)"
  yamdir="YAM${yamver}dev-$(date +%Y%m%d)"
else
  # release build
  yamarcver=$(echo ${yamver} | tr -d ".")
  yamdir="YAM"
fi

# create a fresh archive directory
distdir=$(mktemp -d)

# lets prepare the archive directory
mkdir -p "${distdir}/${yamdir}"

# lets build the debug binary from scratch 
make OS=${yamsys} ${compileflags} distclean all
cp -a src/YAM.${yamsys}.debug "${distdir}/${yamdir}/YAM.debug"
cp -a src/YAM.${yamsys}.map "${distdir}/${yamdir}/YAM.debug.map"
cp -a doc/README.debug "${distdir}/${yamdir}/"

# now we generate an automatic ChangeLog from the git history
if [[ ${LAST_BUILD} -eq 0 ]]; then
  if [[ ${archtype} == "nightly" ]]; then
    since="-10" # last 10 commits
  else
    since="$(git tag -l | grep -e "^[0-9]\.[0-9]*.*[0-9]$" | tail -1)...HEAD" # commits since last release
  fi
else
  since="--since=${LAST_BUILD}"
fi
changelog=$(git log ${since} --pretty=tformat:'%ad %an <%ae>%n%n  %B' --date=short)
if [[ -z ${changelog} ]]; then
  changelog="This is only a rebuild of YAM with no functional changes but updated expiration date only."
fi

echo "${changelog}" >"${distdir}/${yamdir}/ChangeLog"

# cleanup the archive directory from unwanted files
find ${distdir} -nowarn -name ".git" -or -name ".DS_Store" -exec rm -rf {} \; 2>/dev/null

# lets generate the final lha archive
echo "  MK YAM${yamarcver}-${yamarchive}-debug.lha"
rm -f YAM${yamarcver}-${yamarchive}-debug.lha
curdir=$(pwd)
(cd ${distdir}; lha -ao5q "${curdir}/YAM${yamarcver}-${yamarchive}-debug.lha" *)
md5sum "YAM${yamarcver}-${yamarchive}-debug.lha" >"YAM${yamarcver}-${yamarchive}-debug.lha.md5"

# remove the debug binaries again
rm -f "${distdir}/${yamdir}/YAM.debug"
rm -f "${distdir}/${yamdir}/YAM.debug.map"
rm -f "${distdir}/${yamdir}/README.debug"

# copy all common meta documentation files
mkdir -p "${distdir}/${yamdir}/Docs/"
cp -a COPYING "${distdir}/${yamdir}/Docs/"

# copy all stuff common for all platforms
cp -a dist/common/* "${distdir}/"

# copy all stuff common for the platform (e.g. 'aros')
if [[ -e "dist/${yamplatform}" ]]; then
  cp -a dist/${yamplatform}/* "${distdir}/"
fi

# copy all stuff common for the distribution platform (e.g. 'aros-i386)
cp -a dist/${yamsys}/* "${distdir}/"

# lets generate the catalogs (only for nightly+release)
echo "  MK catalogs"
make catalogs

# in case this is a nightly build we have to rename the top-level dir and info icon
if [[ ${archtype} == "nightly" ]]; then
  rsync -a "${distdir}/YAM/" "${distdir}/${yamdir}/"
  rm -rf "${distdir}/YAM"
  mv "${distdir}/YAM.info" "${distdir}/${yamdir}.info"
fi

# move the ChangeLog from the top-level dir to the Docs subdir.
mv "${distdir}/${yamdir}/ChangeLog" "${distdir}/${yamdir}/Docs/"

# copy all translation catalogs
mkdir -p "${distdir}/${yamdir}/Catalogs"
cp -a locale/YAM.pot "${distdir}/${yamdir}/Catalogs/"
for language in $(ls locale/*.catalog); do
  catalog=$(basename "$language")
  lang="${catalog%.*}"
  mkdir -p "${distdir}/${yamdir}/Catalogs/${lang}"
  cp -a ${language} "${distdir}/${yamdir}/Catalogs/${lang}/YAM.catalog"
done

# lets build the release binary from scratch
make OS=${yamsys} DEBUG= ${compileflags} distclean all

# copy the final binary to the release dir
cp -a src/YAM.${yamsys} "${distdir}/${yamdir}/YAM"

# cleanup the archive directory from unwanted files
find ${distdir} -nowarn -name ".git" -or -name ".DS_Store" -exec rm -rf {} \; 2>/dev/null

# lets generate the final lha archive
echo "  MK YAM${yamarcver}-${yamarchive}.lha"
rm -f YAM${yamarcver}-${yamarchive}.lha
curdir=$(pwd)
(cd ${distdir}; lha -ao5q "${curdir}/YAM${yamarcver}-${yamarchive}.lha" *)
md5sum "YAM${yamarcver}-${yamarchive}.lha" >"YAM${yamarcver}-${yamarchive}.lha.md5"

# remove the temporary dist directory
rm -rf "${distdir}"
