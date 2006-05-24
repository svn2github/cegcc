#!/bin/sh
cd $BUILD_DIR
mkdir binutils
cd binutils
$TOP_SRCDIR/src/binutils/configure --prefix=$PREFIX --exec-prefix=$PREFIX --target=arm-wince-pe --disable-nls --includedir=$PREFIX/include || exit 1
make || exit 1
exit 0
