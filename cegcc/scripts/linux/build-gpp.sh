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
$TOP_SRCDIR/src/gcc/configure \
	--prefix=$PREFIX \
	--enable-languages=c,c++ \
	--disable-interwork \
	--disable-nls \
	--enable-checking \
	--disable-multilib \
	--target=$TARGET_ARCH $MY_HOST_ARCH || exit 1

# $TOP_SRCDIR/src/gcc/configure \
# 	--prefix=$PREFIX \
# 	--enable-languages=c,c++ \
# 	--disable-interwork \
# 	--disable-nls \
# 	--enable-checking \
# 	--disable-multilib \
# 	--with-headers \
# 	--target=$TARGET_ARCH $MY_HOST_ARCH || exit 1
#
# It is very important to specify the "all-gcc" target here,
# just making the default target won't work because we're building binutils,
# the compiler and libraries in separate directories.
# And we're doing this so we can upgrade them easier.
#
# make all-gcc || exit 1
#
# This will fail
#
make
#
rm $BUILD_DIR/gpp/gcc/as || exit 1
make || exit 1
exit 0
