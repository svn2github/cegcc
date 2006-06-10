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
# Loop over the list
#
for i in $LIST
do
	install -m 0755 lib$i.a $PREFIX/$TARGET_ARCH/lib
	$TARGET_ARCH-ranlib $PREFIX/$TARGET_ARCH/lib/lib$i.a
done
#
# Set up a libcegcc.a library.
#
mkdir /tmp/libs-$$
cd /tmp/libs-$$
for i in $LIBS_CEGCC_LIB
do
	$TARGET_ARCH-ar x $PREFIX/$TARGET_ARCH/lib/lib$i.a
done
$TARGET_ARCH-ar r $PREFIX/$TARGET_ARCH/lib/libcegcc.a *.o
cd $BUILD_DIR/libs
rm /tmp/libs-$$/*.o
$TARGET_ARCH-ranlib $PREFIX/$TARGET_ARCH/lib/libcegcc.a
#
# End
#
exit 0
