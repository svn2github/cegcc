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
if [ -d $BUILD_DIR/gcc ]; then
	rm -rf $BUILD_DIR/gcc
fi
#
cd $BUILD_DIR
mkdir gcc
cd gcc
#
# This is currently a hack so we don't compile gcc/gcc/config/arm/gthr-win32.c .
# Replaced by --disable-threads.
#
# export CFLAGS="-DCEGCC_LINUX"
#
$TOP_SRCDIR/src/gcc/configure \
	--prefix=$PREFIX \
	--enable-languages=c \
	--disable-interwork \
	--disable-threads \
	--disable-nls \
	--enable-checking \
	--disable-multilib \
	--without-headers \
	--target=$TARGET_ARCH $MY_HOST_ARCH || exit 1
#
# It is very important to specify the "all-gcc" target here,
# just making the default target won't work because we're building binutils,
# the compiler and libraries in separate directories.
# And we're doing this so we can upgrade them easier.
#
make all-gcc || exit 1
exit 0
