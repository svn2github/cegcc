#!/bin/sh
cd $BUILD_DIR
mkdir newlib
cd newlib
#
CC=$PREFIX/bin/$TARGET_ARCH-gcc
LD=$PREFIX/bin/$TARGET_ARCH-ld
CFLAGS="-march=armv4 -DGNUWINCE -DSARM -DWANT_PRINTF_LONG_LONG -DCOMPILING_NEWLIB"
export CC LD CFLAGS
#
$TOP_SRCDIR/src/newlib/newlib/configure --prefix=$PREFIX --target=$TARGET_ARCH $TARGET_ARCH
make
exit 0
