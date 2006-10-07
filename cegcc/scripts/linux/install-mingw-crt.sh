#!/bin/sh
#
# Read the settings
#
if [ -r settings.sh ]; then
	. settings.sh
else
	. scripts/linux/settings.sh
fi
if [ $TGT_ARCH = arm-wince-mingw32ce ]; then
	cd $TOP_SRCDIR/src/mingw-fake_crt || exit 1
	sh install.sh || exit 1
fi
#
exit 0
