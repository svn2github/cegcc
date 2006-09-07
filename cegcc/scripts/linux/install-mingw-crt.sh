#!/bin/sh
#
# Read the settings
#
if [ -r settings.sh ]; then
	. settings.sh
else
	. scripts/linux/settings.sh
fi
cd $TOP_SRCDIR/src/mingw-fake_crt
sh install.sh || exit 1
#
exit 0
