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
# Not in the MingW target
#
if [ $TGT_ARCH = arm-wince-mingw32ce ]; then
	exit 0
fi
cd $BUILD_DIR/gdb || exit 1
#
make install-gdb || exit 1
#
exit 0
