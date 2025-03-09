#!/bin/sh
#
# minor helper script to update the amissl header files
# in our repo

if [ -z $1 ]; then
  echo "ERROR: missing directory path to amissl repo missing"
  exit 2
fi

cp -a $1/include/amissl/*.h include/amissl/
cp -a $1/include/clib/amissl*.h include/clib/
cp -a $1/include/fd/amissl*.fd include/fd/
cp -a $1/include/inline/amissl*.h include/inline/
cp -a $1/include/inline4/amissl*.h include/inline4/
cp -a $1/include/interfaces/amissl*.h include/interfaces/
cp -a $1/include/defines/amissl*.h include/defines/
cp -a $1/include/libraries/amissl*.h include/libraries/
rm -f include/openssl/*.h
cp -a $1/include/openssl/*.h include/openssl/
cp -a $1/include/ppcinline/amissl*.h include/ppcinline/
cp -a $1/include/pragmas/amissl*.h include/pragmas/
cp -a $1/include/sfd/amissl*.sfd include/sfd/
cp -a $1/include/xml/amissl*.xml include/xml/
