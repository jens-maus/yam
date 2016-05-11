#!/bin/sh
############################################################################
#
# YAM - Yet Another Mailer
# Copyright (C) 1995-2000 Marcel Beck
# Copyright (C) 2000-2016 YAM Open Source Team
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

# YAM release build script
# invoke this script as "./mkrelease os4" to build the release archives for AmigaOS4.x

case $1 in
    os3)         yamsys="os3"; yamplatform="os3"; yamarchive="AmigaOS3";;
    os4)         yamsys="os4"; yamplatform="os4"; yamarchive="AmigaOS4";;
    mos)         yamsys="mos"; yamplatform="mos"; yamarchive="MorphOS";;
    aros-i386)   yamsys="aros-i386"; yamplatform="aros"; yamarchive="AROS-i386";;
    aros-ppc)    yamsys="aros-ppc"; yamplatform="aros"; yamarchive="AROS-ppc";;
    aros-x86_64) yamsys="aros-x86_64"; yamplatform="aros"; yamarchive="AROS-x86_64";;
    aros-arm)    yamsys="aros-arm"; yamplatform="aros"; yamarchive="AROS-arm";;
    *)           echo "ERROR: '$1' unknown."; exit 2;;
esac

echo "  MK ${yamsys} release"

yamver=`grep "#define __YAM_VERSION" src/YAM_global.c | cut -d "\"" -f2`
yamarcver=`echo ${yamver} | tr -d "."`

mkdir -p release

# lets generate the catalogs
echo "  MK catalogs"
make catalogs

# lets build everything from scratch 
make OS=${yamsys} DEBUG= DEVFLAGS= clean all

# lets prepare the release directory
rm -rf "release/${yamsys}"
mkdir -p "release/${yamsys}"

# copy all stuff common for all platforms
cp -a dist/common/* "release/${yamsys}/"

# copy all stuff common for the platform (e.g. 'aros')
if [ -e "dist/${yamplatform}" ]; then
  cp -a dist/${yamplatform}/* "release/${yamsys}/"
fi

# copy all stuff common for the distribution platform (e.g. 'aros-i386)
cp -a dist/${yamsys}/* "release/${yamsys}/"

# copy all translation catalogs
mkdir -p "release/${yamsys}/YAM/Catalogs"
cp -a locale/YAM.pot "release/${yamsys}/YAM/Catalogs/"
for language in `ls locale/*.catalog`; do
  catalog=$(basename "$language")
  lang="${catalog%.*}"
  mkdir -p "release/${yamsys}/YAM/Catalogs/${lang}"
  cp -a ${language} "release/${yamsys}/YAM/Catalogs/${lang}/YAM.catalog"
done

# copy the final binary to the release dir
cp -a src/YAM.${yamsys} "release/${yamsys}/YAM/YAM"

# copy all common meta documentation fils
cp -a AUTHORS "release/${yamsys}/YAM/Docs/"
cp -a COPYING "release/${yamsys}/YAM/Docs/"

# generate a readable ChangeLog and copy the file
#cp -a ../ChangeLog "release/$yamsys/YAM $yamver/Docs/ChangeLog"

# lets generate the final distribution/release archive
echo "  MK YAM${yamarcver}-${yamarchive}.lha"
find release/${yamsys} -nowarn -name ".git" -exec rm -rf {} \; 2>/dev/null
cd release/${yamsys}/
lha -ao5q ../YAM${yamarcver}-${yamarchive}.lha *
cd ../../

echo "  MK ${yamsys}-debug"
make OS=$1 DEVFLAGS= clean all

mkdir -p "release/${yamsys}-debug"
cp -a doc/README.debug "release/${yamsys}-debug/"
cp -a src/YAM.${yamsys}.debug "release/${yamsys}-debug/YAM.debug"
cp -a src/YAM.${yamsys}.map "release/${yamsys}-debug/YAM.debug.map"
echo "  MK YAM${yamarcver}-${yamarchive}-debug.lha"
cd release/
lha -ao5q YAM${yamarcver}-${yamarchive}-debug.lha ${yamsys}-debug
cd ../
