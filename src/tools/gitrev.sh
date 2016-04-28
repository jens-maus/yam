#!/bin/bash
#
# This is a helper script that output a git revision identifier
# that allows to identify the git branch and commit SHA of the
# current working copy
#
# Copyright (c) Jens Maus <mail@jens-maus.de>
#

function echo_git_info {
  if [[ $(which git 2> /dev/null) ]]; then
    local STATUS
    STATUS=$(git status 2>/dev/null)
    if [[ -z $STATUS ]]; then
      return
    fi
    echo "`git symbolic-ref HEAD 2> /dev/null | cut -b 12-`-`git log --pretty=format:\"%h\" -1`"
    return
  fi
}

echo "/* Auto-generated file by 'gitrev.sh' */"
echo "#ifndef GIT_REV_H"
echo "#define GIT_REV_H"
echo " #define GIT_REVSTR \"`echo_git_info`\""
echo "#endif"
