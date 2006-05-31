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
# Put the cleanup here instead of in the calling script
#
if [ -d $BUILD_DIR/newlib ]; then
	rm -rf $BUILD_DIR/newlib
fi
#
mkdir -p $BUILD_DIR/newlib
cd $BUILD_DIR/newlib
#
CC=$PREFIX/bin/$TARGET_ARCH-gcc
LD=$PREFIX/bin/$TARGET_ARCH-ld
CFLAGS="-march=armv4 -DGNUWINCE -DSARM -DWANT_PRINTF_LONG_LONG -DCOMPILING_NEWLIB -D__USE_W32_SOCKETS -D_WIN32_WCE=420"
export CC LD CFLAGS
#
$TOP_SRCDIR/src/newlib/newlib/configure --prefix=$PREFIX --target=$TARGET_ARCH $TARGET_ARCH || exit 1
make || exit 1
exit 0
