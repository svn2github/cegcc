#!/bin/sh
cd $BUILD_DIR
mkdir binutils
cd binutils
#
export CFLAGS=""
export LDFLAGS=""
#
$TOP_SRCDIR/src/binutils/configure --prefix=$PREFIX --exec-prefix=$PREFIX --target=arm-wince-pe --disable-nls --includedir=$PREFIX/include || exit 1
make || exit 1
exit 0
