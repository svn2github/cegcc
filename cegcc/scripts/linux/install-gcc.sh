#!/bin/sh
#
# Read the settings
#
if [ -r settings.sh ]; then
	. settings.sh
else
	. scripts/linux/settings.sh
fi
cd $BUILD_DIR/gcc
#
# It is very important to specify the right target here.
#
make install-gcc || exit 1
#
# An apparent problem with the GCC installation
#
GCC_LIBS_LIST="c g m"
for i in $GCC_LIBS_LIST
do
	$TARGET_ARCH-ranlib $PREFIX/$TARGET_ARCH/lib/lib$i.a || exit 1
done
#
exit 0
