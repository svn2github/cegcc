#!/bin/sh
set -x
#
# Read the settings
#
if [ -r settings.sh ]; then
	. settings.sh
else
	. scripts/linux/settings.sh
fi
#
# Put the cleanup here instead of in the calling script
#
if [ -d $BUILD_DIR/gdb ]; then
	rm -rf $BUILD_DIR/gdb
fi
#
mkdir -p $BUILD_DIR/gdb
cd $BUILD_DIR/gdb
#
$TOP_SRCDIR/src/gdb/configure \
	--prefix=$PREFIX \
	--target=$TARGET_ARCH || exit 1
make || exit 1
exit 0
