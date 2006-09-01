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
#if [ -d $BUILD_DIR/libs ]; then
#	rm -rf $BUILD_DIR/libs
#fi
#
# Non-standard build
#
cd $TOP_SRCDIR/src/cegcc/cegccdll
make clean
make PREFIX=$PREFIX
#
# End
#
exit 0
