#!/bin/bash

VER="2.9p1"

for os in AROS-i386 AROS-ppc AROS-x86_64 AmigaOS3 AmigaOS4 MorphOS; do
  echo ${os}
  mkdir -p ${os}/YAM\ ${VER}/Install

  # copy all readme
  cp ../betterstring-mcc/doc/MCC_BetterString.readme ${os}/YAM\ ${VER}/Install/ReadMe-BetterString
  cp ../nlist/docs/ReadMe ${os}/YAM\ ${VER}/Install/ReadMe-NList
  cp ../texteditor-mcc/doc/MCC_TextEditor.readme ${os}/YAM\ ${VER}/Install/ReadMe-TextEditor
  cp ../thebar-mcc/doc/MCC_TheBar.readme ${os}/YAM\ ${VER}/Install/ReadMe-TheBar
  cp ../codesetslib/dist/codesets/ReadMe ${os}/YAM\ ${VER}/Install/ReadMe-codesetslib
  chmod 644 ${os}/YAM\ ${VER}/Install/ReadMe-*

  # copy the binaries
  mkdir -p ${os}/YAM\ ${VER}/Install/MUI
  mkdir -p ${os}/YAM\ ${VER}/Install/Libs
  cp ../betterstring-mcc/release/MCC_BetterString/Libs/MUI/${os}/* ${os}/YAM\ ${VER}/Install/MUI/
  cp ../nlist/release/MCC_NList/Libs/MUI/${os}/* ${os}/YAM\ ${VER}/Install/MUI/
  cp ../texteditor-mcc/release/MCC_TextEditor/Libs/MUI/${os}/* ${os}/YAM\ ${VER}/Install/MUI/
  cp ../thebar-mcc/release/MCC_TheBar/Libs/MUI/${os}/* ${os}/YAM\ ${VER}/Install/MUI/
  cp ../codesetslib/release/codesets/Libs/${os}/* ${os}/YAM\ ${VER}/Install/Libs/

  # copy the charset stuff
  if [ ! "${os}" = "AmigaOS4" ]; then
    cp -R ../codesetslib/release/codesets/Charsets ${os}/YAM\ ${VER}/Install/Libs/
  fi
done
