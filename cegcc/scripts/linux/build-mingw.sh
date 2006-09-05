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
if [ -d $BUILD_DIR/mingw ]; then
	rm -rf $BUILD_DIR/mingw
fi
#
mkdir -p $BUILD_DIR/mingw || exit 1
cd $BUILD_DIR/mingw
#
export CC=$PREFIX/bin/$TGT_ARCH-gcc
export LD=$PREFIX/bin/$TGT_ARCH-ld
export RANLIB=$PREFIX/bin/$TGT_ARCH-ranlib
export DLLTOOL=$PREFIX/bin/$TGT_ARCH-dlltool
export AS=$PREFIX/bin/$TGT_ARCH-as
export CFLAGS="-march=armv4 -DGNUWINCE -D__MINGW32__"
#
$TOP_SRCDIR/src/mingw/configure \
	--prefix=$PREFIX \
	--host=$MY_HOST_ARCH \
	--target=$TGT_ARCH || exit 1
#
make || exit 1
exit 0
