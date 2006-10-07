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
# if [ -d $BUILD_DIR/libs ]; then
# 	rm -rf $BUILD_DIR/libs
# fi
#
# Not in the MingW target
#
if [ $TGT_ARCH = arm-wince-mingw32ce ]; then
	exit 0
fi
#
# Non-standard build
#
cd $TOP_SRCDIR/src/cegcc/cegccthrd || exit 1
make PREFIX=$PREFIX TARGET=$TGT_ARCH install || exit 1
#
cd $TOP_SRCDIR/src/cegcc/libstdc++ || exit 1
make PREFIX=$PREFIX TARGET=$TGT_ARCH install || exit 1
#
# End
#
exit 0
