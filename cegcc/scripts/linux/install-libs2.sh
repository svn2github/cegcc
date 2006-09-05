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
# Non-standard build
#
cd $TOP_SRCDIR/src/cegcc/cegccthrd || exit 1
make PREFIX=$PREFIX install || exit 1
#
cd $TOP_SRCDIR/src/cegcc/libstdc++ || exit 1
make PREFIX=$PREFIX install || exit 1
#
# End
#
exit 0
