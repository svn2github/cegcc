#!/bin/sh
#
# This script can now run in its source directory or from the
# scripts/linux scripts, and either pick up the PREFIX from the
# environment or from the command line.
#
if [ "x$PREFIX" = "x" ]; then
	if [ $# -lt 1 ] ; then
		echo "usage:"
		echo "$0 [prefix dir]"
		exit 1
	fi
	PREFIX=$1
fi

if [ "x$BUILD_DIR" != "x" ] && [ "x$TOP_SRCDIR" != "x" ]; then
	MY_DIR=$BUILD_DIR/mingw-libs
	mkdir -p ${MY_DIR}
	cd $TOP_SRCDIR/src/mingw-fake_crt
else
	MY_DIR=.
fi
TARGET=arm-wince-mingw32ce
LIBDIR=${PREFIX}/${TARGET}/lib

AS=${TARGET}-as
AR=${TARGET}-ar

FAKE_LIBS="libcoldname.a libmingwex.a libmoldname.a libmingw32.a libmingwthrd.a libmoldnamed.a"

${AS} crt2.s -o ${MY_DIR}/crt2.o

mkdir -p ${LIBDIR}
cp -fv ${MY_DIR}/crt2.o ${LIBDIR}/crt2.o

for lib in $FAKE_LIBS; do
   rm -f ${LIBDIR}/$lib
   ${AR} vq ${LIBDIR}/$lib
done
