#!/bin/sh
#
# Read the settings
#
if [ -r settings.sh ]; then
	. settings.sh
else
	. scripts/linux/settings.sh
fi
#
if [ -d $BUILD_DIR/libs ]; then
	rm -rf $BUILD_DIR/libs
fi
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
for i in $BUILD_LIBS_LIST
do
	sh $TOP_SRCDIR/src/cegcc/importlibs/mkimport.sh $TOP_SRCDIR/src/cegcc/importlibs/defs/$i.def . || exit 1
done
#
# End
#
exit 0
