#!/bin/sh
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
# Set up a fake libcegcc.a library.
#
$TARGET_ARCH-ar q $PREFIX/$TARGET_ARCH/lib/libcegcc.a
$TARGET_ARCH-ranlib $PREFIX/$TARGET_ARCH/lib/libcegcc.a
#
# End
#
exit 0
