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
mkdir -p $BUILD_DIR/gpp || exit 1
cd $BUILD_DIR/gpp || exit 1
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
	--build=$MY_HOST_ARCH --target=$TGT_ARCH || exit 1
#
# Deliberately no error handling here.
#
make
#
# Because we need to delete this file
#
rm $BUILD_DIR/gpp/gcc/as || exit 1
#
# Now the build should continue smoothly
#
make || exit 1
exit 0
