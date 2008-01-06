#!/bin/bash -x

if [ "x$PREFIX" = "x" ]; then
	if [ $# -lt 1 ] ; then
		echo "usage:"
		echo "$0 [prefix dir]"
		exit 1
	fi
	PREFIX=$1
fi

if [ x$TARGET = x ]; then
	TARGET=arm-wince-cegcc
fi
LIBDIR=${PREFIX}/${TARGET}/lib

AS=${TARGET}-as
AR=${TARGET}-ar

${AS} cegcc.s -o cegcc.o || exit 1

mkdir -p ${LIBDIR} || exit 1
CEGCCLIB=${LIBDIR}/libcegcc.dll.a

rm -fv ${CEGCCLIB} || exit 1
${TARGET}-ar q ${CEGCCLIB} cegcc.o || exit 1

rm -fv cegcc.o
exit 0
