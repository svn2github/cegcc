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
for TGT_ARCH in arm-wince-cegcc arm-wince-mingw32ce # arm-wince-pe
do
	export TGT_ARCH
#
	sh $SCRIPTDIR/install-binutils.sh || exit 1
	sh $SCRIPTDIR/install-includes.sh || exit 1
	sh $SCRIPTDIR/install-gcc.sh || exit 1
	sh $SCRIPTDIR/install-mingw-crt.sh || exit 1
	sh $SCRIPTDIR/install-newlib.sh || exit 1
	sh $SCRIPTDIR/install-libs.sh || exit 1
	sh $SCRIPTDIR/install-dll.sh || exit 1
	sh $SCRIPTDIR/install-gpp.sh || exit 1
	sh $SCRIPTDIR/install-dll.sh || exit 1
	sh $SCRIPTDIR/install-libs2.sh || exit 1
	sh $SCRIPTDIR/install-gdb.sh || exit 1
	sh $SCRIPTDIR/install-stub.sh || exit 1
#
done
#
sh $SCRIPTDIR/install-docs.sh || exit 1
#
# We should be done now.
#
exit 0
