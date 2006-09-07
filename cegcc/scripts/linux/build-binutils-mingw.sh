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
if [ -d $BUILD_DIR/binutils-mingw ]; then
	rm -rf $BUILD_DIR/binutils-mingw
fi
#
cd $BUILD_DIR
mkdir binutils-mingw || exit 1
cd binutils-mingw
#
export CFLAGS=""
export LDFLAGS=""
#
$TOP_SRCDIR/src/binutils/configure --prefix=$PREFIX \
	--exec-prefix=$PREFIX \
	--target=arm-wince-mingw32 \
	--disable-nls \
	--includedir=$PREFIX/include || exit 1
make || exit 1
exit 0
