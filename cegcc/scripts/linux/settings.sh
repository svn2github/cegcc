#!/bin/sh
if [ -x ./list-libs.sh ]; then
	SCRIPTDIR=`pwd`
	cd ../..
	TOP_SRCDIR=`pwd`
else
	TOP_SRCDIR=`pwd`
	SCRIPTDIR=$TOP_SRCDIR/scripts/linux
fi
#
export SCRIPTDIR TOP_SRCDIR
#
PREFIX=/usr/ppc
export PREFIX
#
# Use a different compiler
#
# export CC=/tmp/gcc-4/bin/gcc
#
MY_HOST_ARCH=`$TOP_SRCDIR/src/gcc/config.guess`
TARGET_ARCH=arm-wince-pe
export MY_HOST_ARCH TARGET_ARCH
#
BUILD_DIR=$TOP_SRCDIR/build/$MY_HOST_ARCH/$TARGET_ARCH
export BUILD_DIR
#
# This set of scripts relies on the fact that $PREFIX/bin is in your path.
# Make sure it is ...
#
export PATH=${PREFIX}/bin:${PATH}
#
# The name of this release
#
RELEASE=0.04-alpha
export RELEASE
#
# The debugging stub
#
export STUB_SRC=${TOP_SRCDIR}/src/gdb/gdb/wince-stub.c
export STUB_EXE=${BUILD_DIR}/gdb/${TARGET_ARCH}-stub.exe

