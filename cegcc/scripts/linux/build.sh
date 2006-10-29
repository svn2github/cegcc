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
# Clean up everything !!
#
if [ -d $BUILD_DIR ]; then
	echo "rm -rf $BUILD_DIR"
	rm -rf $BUILD_DIR
fi
if [ -d $MINGW_BUILD_DIR ]; then
	echo "rm -rf $MINGW_BUILD_DIR"
	rm -rf $MINGW_BUILD_DIR
fi
#
if [ x$PREFIX != x ]; then
	if [ -d $PREFIX -a $PREFIX = /usr/ppc ]; then
		for i in bin info lib man include libexec share arm-wince-pe arm-wince-cegcc arm-wince-mingw32ce
		do
			if [ -d $PREFIX/$i ]; then
				echo "rm -rf $PREFIX/$i"
				rm -rf $PREFIX/$i
			fi
		done
	fi
fi
mkdir -p $BUILD_DIR
#
# The build sequence below should be generic (target-independent).
# In some places, we have a temporary exception (see gpp), in other cases,
# there's a permanent (e.g. by design) difference, such as we don't want to
# distribute newlib in the mingw32ce case. All those differences are hidden
# in the scripts called from this one.
#
# All scripts called below must accept the environment variables
#	TGT_ARCH,
#	BUILD_DIR
# and	PREFIX
# as parameters.
#
# The settings.sh script (called in all of the scripts below)
# uses the TGT_ARCH to determine e.g. BUILD_DIR.
#
for TGT_ARCH in arm-wince-cegcc arm-wince-mingw32ce # arm-wince-pe
do
	export TGT_ARCH
	echo "Running build-arch.sh with TGT_ARCH = " $TGT_ARCH
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
	#
	sh $SCRIPTDIR/install-mingw-crt.sh || exit 1
	sh $SCRIPTDIR/build-libs.sh || exit 1
	sh $SCRIPTDIR/install-libs.sh || exit 1
	#
	if [ $TGT_ARCH = arm-wince-mingw32ce ]; then
		sh $SCRIPTDIR/build-mingw.sh || exit 1
		sh $SCRIPTDIR/install-mingw.sh || exit 1
	else
		sh $SCRIPTDIR/build-newlib.sh || exit 1
		sh $SCRIPTDIR/install-newlib.sh || exit 1
	fi
	#
	# Some of the stuff here applies to both targets
	#
	sh $SCRIPTDIR/build-dll.sh || exit 1
	sh $SCRIPTDIR/install-dll.sh || exit 1
	#
	# Build the compiler better.
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
	sh $SCRIPTDIR/build-mingw.sh || exit 1
	sh $SCRIPTDIR/install-mingw.sh || exit 1
	#
done
#
sh $SCRIPTDIR/install-docs.sh || exit 1
#
# We should be done now.
#
exit 0
