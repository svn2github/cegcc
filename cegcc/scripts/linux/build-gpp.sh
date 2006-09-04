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
if [ -d $BUILD_DIR/gpp ]; then
	rm -rf $BUILD_DIR/gpp
fi
#
cd $BUILD_DIR
mkdir gpp
cd gpp
#
export CFLAGS=""
#
$TOP_SRCDIR/src/gcc/configure \
	--prefix=$PREFIX \
	--enable-languages=c,c++ \
	--disable-interwork \
	--disable-nls \
	--enable-checking \
	--disable-multilib \
	--target=$TARGET_ARCH $MY_HOST_ARCH || exit 1
#
make
#
rm $BUILD_DIR/gpp/gcc/as || exit 1
make || exit 1
exit 0
