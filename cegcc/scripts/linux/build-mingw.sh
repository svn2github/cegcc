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
if [ -d $BUILD_DIR/mingw-runtime ]; then
	rm -rf $BUILD_DIR/mingw-runtime
fi
#
mkdir -p $BUILD_DIR/mingw-runtime || exit 1
cd $BUILD_DIR/mingw-runtime
#
export CC=$PREFIX/bin/$TARGET_ARCH-gcc
export LD=$PREFIX/bin/$TARGET_ARCH-ld
export RANLIB=$PREFIX/bin/$TARGET_ARCH-ranlib
export DLLTOOL=$PREFIX/bin/$TARGET_ARCH-dlltool
export AS=$PREFIX/bin/$TARGET_ARCH-as
export CFLAGS="-march=armv4 -DGNUWINCE -D__MINGW32__"
#
$TOP_SRCDIR/src/mingw-runtime/configure \
	--prefix=$PREFIX \
	--target=$TARGET_ARCH || exit 1
#
make || exit 1
exit 0
