#!/bin/sh
cd $BUILD_DIR
mkdir libs
cd libs
#
# Set up a list of DLL proxies to make.
#
. $SCRIPTDIR/list-libs.sh
#
# Loop over the list
#
for i in $LIST
do
	sh $TOP_SRCDIR/src/cegcc/importlibs/mkimport.sh $TOP_SRCDIR/src/cegcc/importlibs/defs/$i.def .
done
#
# End
#
exit 0
