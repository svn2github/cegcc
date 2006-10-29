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
# Don't do this in MinGW
#
if [ $TGT_ARCH = arm-wince-mingw32ce ]; then
	exit 0
fi
#
cd $BUILD_DIR/newlib || exit 1
#
make install || exit 1
exit 0
