#!/bin/sh
#
# This tiny script takes the YAM.pot file and compares its defined locale string
# with the actual source code of YAM. Found orphaned locale strings will then be
# automatically removed from the .pot file and a cleaned up "YAM.pot" file will be
# written back.
#
# $Id$
#

POTFILE="./YAM.pot"
POFILE="./*.po"
SOURCES="../src/*.c ../src/mui/*.c ../src/rexx/*.c ../src/mime/*.c ../src/tcp/*.c"
TMPCD=`mktemp`

########################################################
# Script starts here
#

if [ "$1" != "" ]; then
  POFILE="$1"
fi

######
# We first check our YAM.cd file for orphaned catalogs IDs
# by scanning through the whole source directory.
echo "Scanning $POTFILE for orphaned catalog IDs:"

# get all currently defined MSG_ tags in the .cd file
MSGTAGS=`awk '/^msgctxt "MSG_.*"/ { print substr($2,2) }' $POTFILE | xargs` 

# now we pick each MSG tag and search in the sources dirs for tr() function
# calls with the same tags
ORPHANEDTAGS=""
for tag in $MSGTAGS; do
  grep -E "$tag[^_[:alnum:]]" $SOURCES | grep -v "_$tag" >/dev/null
  if [ $? -ne 0 ]; then
    echo "$tag not found in source code"
    ORPHANEDTAGS="$ORPHANEDTAGS $tag "
  fi
done

# now we process every orphaned tag and remove it from the YAM.pot file
if [ -n "${ORPHANEDTAGS}" ]; then
  cp $POTFILE $TMPCD.in
  for otag in $ORPHANEDTAGS; do
    awk "BEGIN { tagfound=0; }                \
         {                                    \
           if(tagfound == 1)                  \
           {                                  \
             if(\$0 ~ /^msgctxt \"MSG_.*\"/)  \
             {                                \
               tagfound=0;                    \
             }                                \
             else                             \
             {                                \
               next;                          \
             }                                \
           }                                  \
                                              \
           if(\$0 ~ /^msgctxt \"$otag \(.*\"/)\
           {                                  \
             tagfound=1;                      \
           }                                  \
                                              \
           if(tagfound == 0)                  \
           {                                  \
             print \$0;                       \
           }                                  \
         }" $TMPCD.in > $TMPCD.out
  
    cp $TMPCD.out $TMPCD.in
  done
  
  # copy the processed output file to YAM.cd.new
  cp $TMPCD.out $POTFILE 2>/dev/null
  rm -f $TMPCD.out $TMPCD.in 2>/dev/null
fi
