#!/bin/sh
set -x
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
sh $SCRIPTDIR/build-binutils.sh || exit 1
sh $SCRIPTDIR/install-binutils.sh || exit 1
sh $SCRIPTDIR/build-gcc.sh || exit 1
sh $SCRIPTDIR/install-gcc.sh || exit 1
# sh $SCRIPTDIR/build-includes.sh	This doesn't exist
sh $SCRIPTDIR/install-includes.sh || exit 1
sh $SCRIPTDIR/build-newlib.sh || exit 1
sh $SCRIPTDIR/install-newlib.sh || exit 1
sh $SCRIPTDIR/build-libs.sh || exit 1
sh $SCRIPTDIR/install-libs.sh || exit 1
exit 0
# sh $SCRIPTDIR/build-gdb.sh || exit 1
# sh $SCRIPTDIR/install-gdb.sh || exit 1
exit 0
