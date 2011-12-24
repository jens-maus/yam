#!/bin/sh
#
# This tiny script takes the YAM.cd file and compares its defined locale string
# with the actual source code of YAM. Found orphaned locale strings will then be
# automatically removed from the .cd file and a cleaned up "YAM.cd.new" file will be
# written back to the harddisk.
#
# $Id$
#

CDFILE="./YAM.cd"
CTFILE="./*.ct"
SOURCES="../src/*.c ../src/mui/*.c ../src/rexx/*.c ../src/mime/*.c ../src/tcp/*.c"
TMPCD="/tmp/YAM.cd"

########################################################
# Script starts here
#

if [ "$1" != "" ]; then
  CTFILE="$1"
fi

######
# We first check our YAM.cd file for orphaned catalogs IDs
# by scanning through the whole source directory.
echo "Scanning $CDFILE for orphaned catalog IDs:"

# get all currently defined MSG_ tægs in the .cd file
MSGTAGS=`awk '/^MSG_/ { print $1 }' $CDFILE | xargs` 

# now we pick each MSG tag and search in the sources dirs for GetStr() function
# calls
ORPHANEDTAGS=""
for tag in $MSGTAGS; do
  grep -E "$tag[^_[:alnum:]]" $SOURCES | grep -v "_$tag" >/dev/null
  if [ $? -ne 0 ]; then
    echo "$tag not found in source code"
    ORPHANEDTAGS="$ORPHANEDTAGS $tag "
  fi
done

# now we process every orphaned tag and remove it from the YAM.cd file
cp $CDFILE $TMPCD.in
for otag in $ORPHANEDTAGS; do
  awk "BEGIN { tagfound=0; }        \
       {                            \
         if(tagfound == 1)          \
         {                          \
           if(\$1 ~ /^MSG_/)        \
           {                        \
             tagfound=0;            \
           }                        \
         }                          \
                                    \
         if(\$1 ~ /^$otag$/)        \
         {                          \
           tagfound=1;              \
         }                          \
                                    \
         if(tagfound == 0)          \
         {                          \
           print \$0;               \
         }                          \
       }" $TMPCD.in > $TMPCD.out

  cp $TMPCD.out $TMPCD.in
done

# copy the processed output file to YAM.cd.new
cp $TMPCD.out $CDFILE 2>/dev/null
rm $TMPCD.out $TMPCD.in 2>/dev/null

######
# Now we check all existing catalog translations and output
# which orphaned catalogs IDs which can be deleted
CTFILES=`ls ${CTFILE}`
for ctfile in $CTFILES; do
  echo ""
  echo "Scanning $ctfile for orphaned catalogs IDs:"
  CTTAGS=`awk '/^MSG_/ { print $1 }' $ctfile | xargs` 
  for cttag in $CTTAGS; do
    echo "$MSGTAGS " | grep -E "$cttag " >/dev/null
    if [ $? -ne 0 ]; then
      echo "'$cttag' is orphaned"
    fi
  done
done

######
# Now we check all existing catalog translations and output
# which catalogs IDs are actually missing
for ctfile in $CTFILES; do
  echo ""
  echo "Scanning $ctfile for missing catalogs IDs:"
  for tag in $MSGTAGS; do
    grep -E "$tag" $ctfile >/dev/null
    if [ $? -ne 0 ]; then
      echo "'$tag' is missing"
    fi
  done
done
