#!/bin/bash

if [[ ! -e dist/common ]]; then
  echo "ERROR: This script has to be called from the top-level directory."
  exit 2
fi

# make a temp directory
TMPDIR=$(mktemp -d)

# lets download the latest release archives
wget -O ${TMPDIR}/bstring.lha $(curl -s https://api.github.com/repos/amiga-mui/betterstring/releases | grep browser_download_url | head -n 1 | cut -d '"' -f 4)
wget -O ${TMPDIR}/nlist.lha $(curl -s https://api.github.com/repos/amiga-mui/nlist/releases | grep browser_download_url | head -n 1 | cut -d '"' -f 4)
wget -O ${TMPDIR}/texteditor.lha $(curl -s https://api.github.com/repos/amiga-mui/texteditor/releases | grep browser_download_url | head -n 1 | cut -d '"' -f 4)
wget -O ${TMPDIR}/thebar.lha $(curl -s https://api.github.com/repos/amiga-mui/thebar/releases | grep browser_download_url | head -n 1 | cut -d '"' -f 4)
wget -O ${TMPDIR}/libcodesets.lha $(curl -s https://api.github.com/repos/jens-maus/libcodesets/releases | grep browser_download_url | head -n 1 | cut -d '"' -f 4)

# lets extract the necessary files
lha xw=${TMPDIR} ${TMPDIR}/bstring.lha MCC_BetterString/ReadMe MCC_BetterString/Libs/MUI/*
lha xw=${TMPDIR} ${TMPDIR}/nlist.lha MCC_NList/ReadMe MCC_NList/Libs/MUI/*
lha xw=${TMPDIR} ${TMPDIR}/texteditor.lha MCC_TextEditor/ReadMe MCC_TextEditor/Libs/MUI/*
lha xw=${TMPDIR} ${TMPDIR}/thebar.lha MCC_TheBar/ReadMe MCC_TheBar/Libs/MUI/*
lha xw=${TMPDIR} ${TMPDIR}/libcodesets.lha codesets/ReadMe codesets/Libs/* codesets/Charsets/*

# lets copy the readme files to the 'dist' directory.
cp ${TMPDIR}/MCC_BetterString/ReadMe dist/common/YAM/Install/ReadMe-BetterString
cp ${TMPDIR}/MCC_NList/ReadMe dist/common/YAM/Install/ReadMe-NList
cp ${TMPDIR}/MCC_TextEditor/ReadMe dist/common/YAM/Install/ReadMe-TextEditor
cp ${TMPDIR}/MCC_TheBar/ReadMe dist/common/YAM/Install/ReadMe-TheBar
cp ${TMPDIR}/codesets/ReadMe dist/common/YAM/Install/ReadMe-codesetslib

# copy the MCC binaries to the 'dist' directory.
cp ${TMPDIR}/MCC_*/Libs/MUI/AmigaOS3/* dist/os3/YAM/Install/MUI/
cp ${TMPDIR}/MCC_*/Libs/MUI/AmigaOS4/* dist/os4/YAM/Install/MUI/
cp ${TMPDIR}/MCC_*/Libs/MUI/MorphOS/* dist/mos/YAM/Install/MUI/
cp ${TMPDIR}/MCC_*/Libs/MUI/AROS-i386/* dist/aros-i386/YAM/Install/MUI/
cp ${TMPDIR}/MCC_*/Libs/MUI/AROS-ppc/* dist/aros-ppc/YAM/Install/MUI/
cp ${TMPDIR}/MCC_*/Libs/MUI/AROS-x86_64/* dist/aros-x86_64/YAM/Install/MUI/

# copy codesetslib stuff to 'dist' directory.
cp ${TMPDIR}/codesets/Libs/AmigaOS3/* dist/os3/YAM/Install/Libs/
cp ${TMPDIR}/codesets/Libs/AmigaOS4/* dist/os4/YAM/Install/Libs/
cp ${TMPDIR}/codesets/Libs/MorphOS/* dist/mos/YAM/Install/Libs/
cp ${TMPDIR}/codesets/Libs/AROS-i386/* dist/aros-i386/YAM/Install/Libs/
cp ${TMPDIR}/codesets/Libs/AROS-ppc/* dist/aros-ppc/YAM/Install/Libs/
cp ${TMPDIR}/codesets/Libs/AROS-x86_64/* dist/aros-x86_64/YAM/Install/Libs/
cp ${TMPDIR}/codesets/Charsets/* dist/os3/YAM/Install/Libs/Charsets/
cp ${TMPDIR}/codesets/Charsets/* dist/mos/YAM/Install/Libs/Charsets/
cp ${TMPDIR}/codesets/Charsets/* dist/aros/YAM/Install/Libs/Charsets/

# remove temporary directory
rm -rf ${TMPDIR}

exit 0
