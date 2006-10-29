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
# Put the cleanup here instead of in the calling script
#
if [ -d $BUILD_DIR/newlib ]; then
	rm -rf $BUILD_DIR/newlib
fi
#
mkdir -p $BUILD_DIR/newlib || exit 1
cd $BUILD_DIR/newlib || exit 1
#
export CC=$PREFIX/bin/$TGT_ARCH-gcc
export LD=$PREFIX/bin/$TGT_ARCH-ld
export RANLIB=$PREFIX/bin/$TGT_ARCH-ranlib
export CFLAGS="-march=armv4 -DGNUWINCE -DSARM -DWANT_PRINTF_LONG_LONG -DCOMPILING_NEWLIB -D_WIN32_WCE=420"
#
$TOP_SRCDIR/src/newlib/newlib/configure \
	--prefix=$PREFIX \
	--target=$TGT_ARCH $TGT_ARCH || exit 1
#
make || exit 1
exit 0
