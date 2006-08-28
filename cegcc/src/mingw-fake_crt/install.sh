#!/bin/sh

PREFIX=/usr/local/mingw32ce
TARGET=arm-wince-mingw32
LIBDIR=${PREFIX}/${TARGET}/lib

AS=${TARGET}-as
AR=${TARGET}-ar

FAKE_LIBS="libcoldname.a libmingwex.a libmoldname.a libmingw32.a libmingwthrd.a libmoldnamed.a"

${AS} crt2.s -o crt2.o
cp -fv crt2.o ${LIBDIR}/crt2.o

for lib in $FAKE_LIBS; do
   rm -f ${LIBDIR}/$lib
   ${AR} vq ${LIBDIR}/$lib
done
