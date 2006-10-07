#!/bin/sh
#
# Read the settings
#
if [ -r settings.sh ]; then
	. settings.sh
else
	. scripts/linux/settings.sh
fi
cd $BUILD_DIR/libs
#
# Set up a list of DLL proxies to make.
#
. $SCRIPTDIR/list-libs.sh
#
# Create directory
#
mkdir -p $PREFIX/$TGT_ARCH/lib || exit 1
#
# Loop over the list
#
for i in $INSTALL_PE_TGT
do
	install -m 0755 lib$i.a $PREFIX/$TGT_ARCH/lib || exit 1
	$TGT_ARCH-ranlib $PREFIX/$TGT_ARCH/lib/lib$i.a || exit 1
done
# for i in $INSTALL_MINGW_TGT
# do
# 	install -m 0755 lib$i.a $PREFIX/$MINGW_TGT_ARCH/lib || exit 1
# 	$MINGW_TGT_ARCH-ranlib $PREFIX/$MINGW_TGT_ARCH/lib/lib$i.a || exit 1
# done
#
# End
#
exit 0
