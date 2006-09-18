#!/bin/sh
set -x
#
# Read the settings
#
if [ -r settings.sh ]; then
	. settings.sh
else
	. scripts/linux/settings.sh
fi
#
# Clean up everything !!
#
if [ -d $BUILD_DIR ]; then
	if [ -d $TOP_SRCDIR/build ]; then
		echo "rm -rf $TOP_SRCDIR/build"
		rm -rf $TOP_SRCDIR/build
	fi
fi
#
if [ x$PREFIX != x ]; then
	if [ -d $PREFIX -a $PREFIX = /usr/ppc ]; then
		for i in $TGT_ARCH bin info lib man include libexec share
		do
			if [ -d $PREFIX/$i ]; then
				echo "rm -rf $PREFIX/$i"
				rm -rf $PREFIX/$i
			fi
		done
	fi
fi
if [ x$MY_RPM_PREFIX != x ]; then
	if [ -d $MY_RPM_PREFIX ]; then
		rm -rf $MY_RPM_PREFIX
	fi
fi
mkdir -p $BUILD_DIR
#
sh $SCRIPTDIR/build-binutils.sh || exit 1
sh $SCRIPTDIR/install-binutils.sh || exit 1
#
# There's no 'build-includes' script.
#
sh $SCRIPTDIR/install-includes.sh || exit 1
#
# Bootstrap the C compiler, we need it to get newlib into shape,
# and that in turn is a prereq to get the C/C++ compiler built completely.
#
sh $SCRIPTDIR/build-gcc.sh || exit 1
sh $SCRIPTDIR/install-gcc.sh || exit 1
sh $SCRIPTDIR/build-newlib.sh || exit 1
sh $SCRIPTDIR/install-newlib.sh || exit 1
#
# Also build the other target : arm-wince-mingw32
#
sh $SCRIPTDIR/build-binutils-mingw.sh || exit 1
sh $SCRIPTDIR/install-binutils-mingw.sh || exit 1
sh $SCRIPTDIR/build-gcc-mingw.sh || exit 1
sh $SCRIPTDIR/install-gcc-mingw.sh || exit 1
sh $TOP_SRCDIR/src/mingw-fake_crt/install.sh || exit 1
#
# Some of the stuff here applies to both targets
#
sh $SCRIPTDIR/build-libs.sh || exit 1
sh $SCRIPTDIR/install-libs.sh || exit 1
#
# Bootstrap the cegcc lib too
#
sh $SCRIPTDIR/build-dll.sh || exit 1
sh $SCRIPTDIR/install-dll.sh || exit 1
#
# Build the compiler better (not needed for mingw?).
#
sh $SCRIPTDIR/build-gpp.sh || exit 1
sh $SCRIPTDIR/install-gpp.sh || exit 1
#
# This must happen with the complete compiler to have threads support.
#
sh $SCRIPTDIR/build-dll.sh || exit 1
sh $SCRIPTDIR/install-dll.sh || exit 1
#
# These depend on the better compiler
#
sh $SCRIPTDIR/build-libs2.sh || exit 1
sh $SCRIPTDIR/install-libs2.sh || exit 1
#
sh $SCRIPTDIR/build-gdb.sh || exit 1
sh $SCRIPTDIR/install-gdb.sh || exit 1
sh $SCRIPTDIR/build-stub.sh || exit 1
sh $SCRIPTDIR/install-stub.sh || exit 1
#
sh $SCRIPTDIR/install-docs.sh || exit 1
#
# We should be done now.
#
exit 0
