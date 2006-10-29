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
# Temporary hack - not in the MingW target
# FIX ME
#
# if [ $TGT_ARCH = arm-wince-mingw32ce ]; then
# 	exit 0
# fi
cd $BUILD_DIR/gpp || exit 1
#
make install || exit 1
#
exit 0
