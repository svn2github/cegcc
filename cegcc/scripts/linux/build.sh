#!/bin/sh
. settings.sh
#
# Clean up everything !!
#
if [ -d $BUILD_DIR ]; then
	if [ -d $TOP_SRCDIR/build ]; then
		rm -rf $TOP_SRCDIR/build
	fi
fi
mkdir -p $BUILD_DIR
#
sh $SCRIPTDIR/build-binutils.sh
sh $SCRIPTDIR/install-binutils.sh
sh $SCRIPTDIR/build-gcc.sh
sh $SCRIPTDIR/install-gcc.sh
sh $SCRIPTDIR/build-newlib.sh
sh $SCRIPTDIR/install-newlib.sh
sh $SCRIPTDIR/build-libs.sh
sh $SCRIPTDIR/install-libs.sh
# sh $SCRIPTDIR/build-includes.sh	This doesn't exist
sh $SCRIPTDIR/install-includes.sh
# sh $SCRIPTDIR/build-gdb.sh
# sh $SCRIPTDIR/install-gdb.sh
exit 0
