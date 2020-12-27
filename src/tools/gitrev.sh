#!/bin/bash
#
# This is a helper script that output a git revision identifier
# that allows to identify the git branch and commit SHA of the
# current working copy
#
# Copyright (c) Jens Maus <mail@jens-maus.de>
#

function echo_git_info {
  if [[ $(which git 2>/dev/null) ]]; then
    if [[ -z $(git status 2>/dev/null) ]]; then
      return
    fi
    BRANCH=$(git symbolic-ref HEAD 2>/dev/null | cut -b 12-)
    COMMIT=$(git log --pretty=format:%h -1 | head -c7)
    if [[ ${BRANCH} == "master" ]]; then
      echo "${COMMIT}"
    else
      echo "${BRANCH}-${COMMIT}"
    fi
    return
  fi
  return
}

# we output a whole c-like header file in case
# arguments are given to gitrev.sh
if [[ $# -gt 0 ]]; then
  echo "/* Auto-generated file by 'gitrev.sh' */"
  echo "#ifndef GIT_REV_H"
  echo "#define GIT_REV_H"
  echo " #define GIT_REVSTR \"$(echo_git_info)\""
  echo "#endif"
else
  # otherwise we output the git revision only.
  echo_git_info
fi
