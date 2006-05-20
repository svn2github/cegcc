#!/bin/sh
cd $BUILD_DIR
mkdir binutils
cd binutils
$TOP_SRCDIR/src/binutils/configure --prefix=$PREFIX --exec-prefix=$PREFIX --target=arm-wince-pe --disable-nls --includedir=$PREFIX/include
make
exit 0
