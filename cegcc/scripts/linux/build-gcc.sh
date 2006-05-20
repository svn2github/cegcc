#!/bin/sh
cd $BUILD_DIR
mkdir gcc
cd gcc
# $TOP_SRCDIR/src/gcc/configure \
# 	--prefix=$PREFIX \
# 	--enable-languages=c,c++ \
# 	--disable-multilib \
# 	--without-headers \
# 	--disable-interwork \
# 	--with-newlib \
# 	--enable-checking \
# 	--with-gcc \
# 	--with-gnu-ld \
# 	--with-gnu-as \
# 	--target=$TARGET_ARCH $MY_HOST_ARCH
$TOP_SRCDIR/src/gcc/configure \
	--prefix=$PREFIX \
	--enable-languages=c,c++ \
	--disable-multilib \
	--without-headers \
	--disable-interwork \
	--with-newlib \
	--enable-checking \
	--target=$TARGET_ARCH $MY_HOST_ARCH
#
# It is very important to specify the "all-gcc" target here,
# just making the default target won't work because we're building binutils,
# the compiler and libraries in separate directories.
# And we're doing this so we can upgrade them easier.
#
make all-gcc
exit 0
