#!/bin/sh

############################################################################
#
# YAM - Yet Another Mailer
# Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
# Copyright (C) 2000-2010 by YAM Open Source Team
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
    os3)         yamsys="AmigaOS3";;
    os4)         yamsys="AmigaOS4";;
    mos)         yamsys="MorphOS";;
    aros-i386)   yamsys="AROS-i386";;
    aros-ppc)    yamsys="AROS-ppc";;
    aros-x86_64) yamsys="AROS-x86_64";;
esac
echo "  MK $yamsys release"

yamver="2.6"
yamarcver="26"

mkdir -p release

echo "  MK catalogs"
make catalogs

make OS=$1 DEBUG= DEVFLAGS= clean
make OS=$1 DEBUG= DEVFLAGS= all
rm -rf "release/$yamsys"
mkdir -p "release/$yamsys"
mkdir -p "release/$yamsys/YAM $yamver"
cp ../icons/$1/YAM_directory.info "release/$yamsys/YAM $yamver.info"
echo "  MK $yamsys/Catalogs"
mkdir -p "release/$yamsys/YAM $yamver/Catalogs"
cp ../icons/$1/Catalogs_directory.info "release/$yamsys/YAM $yamver/Catalogs.info"
cp ../locale/YAM.cd "release/$yamsys/YAM $yamver/Catalogs/YAM.cd"
for language in czech dutch english-british french german greek italian polish swedish; do
	mkdir -p "release/$yamsys/YAM $yamver/Catalogs/$language"
	cp ../locale/$language.catalog "release/$yamsys/YAM $yamver/Catalogs/$language/YAM.catalog"
done
echo "  MK $yamsys/Docs"
mkdir -p "release/$yamsys/YAM $yamver/Docs"
cp ../icons/$1/Docs_directory.info "release/$yamsys/YAM $yamver/Docs.info"
cp ../doc/YAM_english.guide "release/$yamsys/YAM $yamver/Docs/YAM_english.guide"
cp ../icons/$1/guide.info "release/$yamsys/YAM $yamver/Docs/YAM_english.guide.info"
cp ../doc/YAM_french.guide "release/$yamsys/YAM $yamver/Docs/YAM_french.guide"
cp ../icons/$1/guide.info "release/$yamsys/YAM $yamver/Docs/YAM_french.guide.info"
cp ../AUTHORS "release/$yamsys/YAM $yamver/Docs/AUTHORS"
cp ../icons/$1/Docs_AUTHORS.info "release/$yamsys/YAM $yamver/Docs/AUTHORS.info"
cp ../ChangeLog "release/$yamsys/YAM $yamver/Docs/ChangeLog"
cp ../icons/$1/Docs_ChangeLog.info "release/$yamsys/YAM $yamver/Docs/ChangeLog.info"
cp ../COPYING "release/$yamsys/YAM $yamver/Docs/COPYING"
cp ../icons/$1/Docs_COPYING.info "release/$yamsys/YAM $yamver/Docs/COPYING.info"
echo "  MK $yamsys/Gallery"
mkdir -p "release/$yamsys/YAM $yamver/Gallery"
cp ../icons/$1/Gallery_directory.info "release/$yamsys/YAM $yamver/Gallery.info"
cp ../doc/ReadMe_gallery "release/$yamsys/YAM $yamver/Gallery/ReadMe"
cp ../icons/$1/ReadMe.info "release/$yamsys/YAM $yamver/Gallery/ReadMe.info"
echo "  MK $yamsys/Install"
mkdir -p "release/$yamsys/YAM $yamver/Install"
cp -r "../../distribution/$yamsys/YAM $yamver/Install/" "release/$yamsys/YAM $yamver/"
cp ../icons/$1/Install_directory.info "release/$yamsys/YAM $yamver/Install.info"
cp ../doc/Install-YAM "release/$yamsys/YAM $yamver/Install/Install-YAM"
cp ../icons/$1/Install-YAM.info "release/$yamsys/YAM $yamver/Install/Install-YAM.info"
echo "  MK $yamsys/Rexx"
mkdir -p "release/$yamsys/YAM $yamver/Rexx"
cp -r ../rexx/* "release/$yamsys/YAM $yamver/Rexx/"
cp ../icons/$1/Rexx_directory.info "release/$yamsys/YAM $yamver/Rexx.info"
echo "  MK $yamsys/Themes"
mkdir -p "release/$yamsys/YAM $yamver/Themes"
cp -r ../themes/* "release/$yamsys/YAM $yamver/Themes/"
cp ../icons/$1/Themes_directory.info "release/$yamsys/YAM $yamver/Themes.info"
cp ../icons/$1/Themes_ReadMe.info "release/$yamsys/YAM $yamver/Themes/ReadMe.info"
cp YAM.$1 "release/$yamsys/YAM $yamver/YAM"
cp ../icons/$1/YAM.info "release/$yamsys/YAM $yamver/YAM.info"
cp ../doc/ReadMe "release/$yamsys/YAM $yamver/ReadMe"
cp ../icons/$1/ReadMe.info "release/$yamsys/YAM $yamver/ReadMe.info"
cp ../doc/.addressbook "release/$yamsys/YAM $yamver/.addressbook"
cp ../doc/.taglines "release/$yamsys/YAM $yamver/.taglines"
echo "  MK YAM$yamarcver-$yamsys.lha"
find release/$yamsys -nowarn -name ".svn" -exec rm -rf {} \; 2>/dev/null
cd release/$yamsys/
lha -aq ../YAM$yamarcver-$yamsys.lha *
cd ../../

echo "  MK $yamsys-debug"
make OS=$1 DEVFLAGS= clean
make OS=$1 DEVFLAGS= all
cp ../doc/README.debug "release/"
cp YAM.$1.debug "release/YAM.debug"
cp YAM.$1.map "release/YAM.debug.map"
echo "  MK YAM$yamarcver-$yamsys-debug.lha"
cd release
lha -aq YAM$yamarcver-$yamsys-debug.lha README.debug YAM.debug YAM.debug.map
cd ../
