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
sh $SCRIPTDIR/install-binutils.sh || exit 1
sh $SCRIPTDIR/install-includes.sh || exit 1
sh $SCRIPTDIR/install-newlib.sh || exit 1
sh $SCRIPTDIR/install-libs.sh || exit 1
sh $SCRIPTDIR/install-dll.sh || exit 1
sh $SCRIPTDIR/install-gdb.sh || exit 1
sh $SCRIPTDIR/install-stub.sh || exit 1
sh $SCRIPTDIR/install-gpp.sh || exit 1
#
# We should be done now.
#
exit 0
