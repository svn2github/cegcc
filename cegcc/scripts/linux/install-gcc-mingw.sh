#!/bin/sh
#
# Read the settings
#
if [ -r settings.sh ]; then
	. settings.sh
else
	. scripts/linux/settings.sh
fi
cd $BUILD_DIR/gcc-mingw
#
# It is very important to specify the right target here.
#
make install-gcc || exit 1
#
exit 0
