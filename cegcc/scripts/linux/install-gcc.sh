#!/bin/sh
cd $BUILD_DIR/gcc
#
# It is very important to specify the right target here.
#
make install-gcc
#
# An apparent problem with the GCC installation
#
GCC_LIBS_LIST="c a m"
for i in $GCC_LIBS_LIST
do
	$TARGET_ARCH-ranlib $PREFIX/$TARGET_ARCH/lib/lib$i.a
done
#
exit 0
