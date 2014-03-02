#!/bin/sh

############################################################################
#
# YAM - Yet Another Mailer
# Copyright (C) 1995-2000 Marcel Beck
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
# YAM Official Support Site :  http://www.yam.ch
# YAM OpenSource project    :  http://sourceforge.net/projects/yamos/
#
# $Id$
#
############################################################################

# YAM release build script
# invoke this script as "./mkrelease os4" to build the release archives for AmigaOS4.x

case $1 in
    os3)         yamsys="AmigaOS3"; yamicons="os3";;
    os4)         yamsys="AmigaOS4"; yamicons="os4";;
    mos)         yamsys="MorphOS"; yamicons="mos";;
    aros-i386)   yamsys="AROS-i386"; yamicons="aros";;
    aros-ppc)    yamsys="AROS-ppc"; yamicons="aros";;
    aros-x86_64) yamsys="AROS-x86_64"; yamicons="aros";;
    aros-arm)    yamsys="AROS-arm"; yamicons="aros";;
esac
echo "  MK $yamsys release"

yamver=`grep "#define __YAM_VERSION" YAM_global.c | cut -d "\"" -f2`
yamarcver=`echo ${yamver} | tr -d "."`

mkdir -p release

echo "  MK catalogs"
make catalogs

make OS=$1 DEBUG= DEVFLAGS= clean
make OS=$1 DEBUG= DEVFLAGS= all
rm -rf "release/$yamsys"
mkdir -p "release/$yamsys"
mkdir -p "release/$yamsys/YAM $yamver"
cp -a ../icons/$yamicons/YAM_directory.info "release/$yamsys/YAM $yamver.info"
echo "  MK $yamsys/Catalogs"
mkdir -p "release/$yamsys/YAM $yamver/Catalogs"
cp -a ../icons/$yamicons/Catalogs_directory.info "release/$yamsys/YAM $yamver/Catalogs.info"
cp -a ../locale/YAM.cd "release/$yamsys/YAM $yamver/Catalogs/YAM.cd"
for language in `ls ../locale/*.catalog`; do
  catalog=$(basename "$language")
  lang="${catalog%.*}"
  mkdir -p "release/$yamsys/YAM $yamver/Catalogs/${lang}"
  cp -a ${language} "release/$yamsys/YAM $yamver/Catalogs/${lang}/YAM.catalog"
done
echo "  MK $yamsys/Docs"
mkdir -p "release/$yamsys/YAM $yamver/Docs"
cp -a ../icons/$yamicons/Docs_directory.info "release/$yamsys/YAM $yamver/Docs.info"
#for language in english french german spanish; do
#	cp -a ../doc/YAM_$language.guide "release/$yamsys/YAM $yamver/Docs/YAM_$language.guide"
#	cp -a ../icons/$yamicons/guide.info "release/$yamsys/YAM $yamver/Docs/YAM_$language.guide.info"
#done
cp -a ../AUTHORS "release/$yamsys/YAM $yamver/Docs/AUTHORS"
cp -a ../icons/$yamicons/Docs_AUTHORS.info "release/$yamsys/YAM $yamver/Docs/AUTHORS.info"
cp -a ../ChangeLog "release/$yamsys/YAM $yamver/Docs/ChangeLog"
cp -a ../icons/$yamicons/Docs_ChangeLog.info "release/$yamsys/YAM $yamver/Docs/ChangeLog.info"
cp -a ../COPYING "release/$yamsys/YAM $yamver/Docs/COPYING"
cp -a ../icons/$yamicons/Docs_COPYING.info "release/$yamsys/YAM $yamver/Docs/COPYING.info"
echo "  MK $yamsys/Install"
mkdir -p "release/$yamsys/YAM $yamver/Install"
svn --force export "https://svn.yam.ch/distribution/$yamsys/YAM%20$yamver/Install/" "release/$yamsys/YAM $yamver/Install/"
cp -a ../icons/$yamicons/Install_directory.info "release/$yamsys/YAM $yamver/Install.info"
cp -a ../doc/Install-YAM "release/$yamsys/YAM $yamver/Install/Install-YAM"
cp -a ../icons/$yamicons/Install-YAM.info "release/$yamsys/YAM $yamver/Install/Install-YAM.info"
echo "  MK $yamsys/Resources"
mkdir -p "release/$yamsys/YAM $yamver/Resources"
cp -a ../icons/$yamicons/Resources_directory.info "release/$yamsys/YAM $yamver/Resources.info"
echo "  MK $yamsys/Resources/Certificates"
mkdir -p "release/$yamsys/YAM $yamver/Resources/Certificates"
cp -a ../resources/certificates/ca-bundle.crt "release/$yamsys/YAM $yamver/Resources/Certificates/ca-bundle.crt"
cp -a ../icons/$yamicons/Certificates_directory.info "release/$yamsys/YAM $yamver/Resources/Certificates.info"
echo "  MK $yamsys/Resources/Gallery"
mkdir -p "release/$yamsys/YAM $yamver/Resources/Gallery"
cp -a ../icons/$yamicons/Gallery_directory.info "release/$yamsys/YAM $yamver/Resources/Gallery.info"
cp -a ../doc/ReadMe_gallery "release/$yamsys/YAM $yamver/Resources/Gallery/ReadMe"
cp -a ../icons/$yamicons/ReadMe.info "release/$yamsys/YAM $yamver/Resources/Gallery/ReadMe.info"
echo "  MK $yamsys/Resources/Spamfilters"
mkdir -p "release/$yamsys/YAM $yamver/Resources/Spamfilters"
cp -a ../resources/spamfilters/*.sfd "release/$yamsys/YAM $yamver/Resources/Spamfilters/"
cp -a ../icons/$yamicons/Spamfilters_directory.info "release/$yamsys/YAM $yamver/Resources/Spamfilters.info"
echo "  MK $yamsys/Resources/Themes"
mkdir -p "release/$yamsys/YAM $yamver/Resources/Themes"
cp -ra ../resources/themes/* "release/$yamsys/YAM $yamver/Resources/Themes/"
cp -a ../icons/$yamicons/Themes_directory.info "release/$yamsys/YAM $yamver/Resources/Themes.info"
cp -a ../icons/$yamicons/Themes_ReadMe.info "release/$yamsys/YAM $yamver/Resources/Themes/ReadMe.info"
echo "  MK $yamsys/Resources/Zoneinfo"
mkdir -p "release/$yamsys/YAM $yamver/Resources/Zoneinfo"
cp -ra ../resources/zoneinfo/* "release/$yamsys/YAM $yamver/Resources/Zoneinfo/"
cp -a ../icons/$yamicons/Zoneinfo_directory.info "release/$yamsys/YAM $yamver/Resources/Zoneinfo.info"
echo "  MK $yamsys/Rexx"
mkdir -p "release/$yamsys/YAM $yamver/Rexx"
cp -ra ../rexx/* "release/$yamsys/YAM $yamver/Rexx/"
cp -a ../icons/$yamicons/Rexx_directory.info "release/$yamsys/YAM $yamver/Rexx.info"
cp -a YAM.$1 "release/$yamsys/YAM $yamver/YAM"
cp -a ../icons/$yamicons/YAM.info "release/$yamsys/YAM $yamver/YAM.info"
cp -a ../doc/ReadMe "release/$yamsys/YAM $yamver/ReadMe"
cp -a ../icons/$yamicons/ReadMe.info "release/$yamsys/YAM $yamver/ReadMe.info"
cp -a ../doc/.addressbook "release/$yamsys/YAM $yamver/.addressbook"
cp -a ../doc/.taglines "release/$yamsys/YAM $yamver/.taglines"
echo "  MK YAM$yamarcver-$yamsys.lha"
find release/$yamsys -nowarn -name ".svn" -exec rm -rf {} \; 2>/dev/null
cd release/$yamsys/
lha -ao5q ../YAM$yamarcver-$yamsys.lha *
cd ../../

echo "  MK $yamsys-debug"
make OS=$1 DEVFLAGS= clean
make OS=$1 DEVFLAGS= all
cp -a ../doc/README.debug "release/"
cp -a YAM.$1.debug "release/YAM.debug"
cp -a YAM.$1.map "release/YAM.debug.map"
echo "  MK YAM$yamarcver-$yamsys-debug.lha"
cd release
lha -ao5q YAM$yamarcver-$yamsys-debug.lha README.debug YAM.debug YAM.debug.map
cd ../
