#!/bin/sh
# HACK
# export TGT_ARCH=arm-wince-mingw32ce
#
# Read the settings
#
if [ -r settings.sh ]; then
	. settings.sh
else
	. scripts/linux/settings.sh
fi
#
#
if [ $TGT_ARCH = "arm-wince-mingw32ce" ]; then
	cd $BUILD_DIR/profile || exit 1
	make install || exit 1
else
	cd $BUILD_DIR/profile/profile || exit 1
	cp libgmon.a gcrt1.o gcrt2.o ${PREFIX}/${TGT_ARCH}/lib
	${TGT_ARCH}-ranlib ${PREFIX}/${TGT_ARCH}/lib/libgmon.a 
fi
#
exit 0
