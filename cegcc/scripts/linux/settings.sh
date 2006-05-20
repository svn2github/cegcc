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
MY_HOST_ARCH=`$TOP_SRCDIR/src/gcc/config.guess`
TARGET_ARCH=arm-wince-pe
export MY_HOST_ARCH TARGET_ARCH
#
BUILD_DIR=$TOP_SRCDIR/build/$MY_HOST_ARCH/$TARGET_ARCH
export BUILD_DIR
