#!/bin/bash -x

if [ "x$PREFIX" = "x" ]; then
	if [ $# -lt 1 ] ; then
		echo "usage:"
		echo "$0 [prefix dir]"
		exit 1
	fi
	PREFIX=$1
fi

TARGET=arm-wince-cegcc
LIBDIR=${PREFIX}/${TARGET}/lib

AS=${TARGET}-as
AR=${TARGET}-ar

${AS} cegcc.s -o cegcc.o

mkdir -p ${LIBDIR}
CEGCCLIB=${LIBDIR}/libcegcc.dll.a

rm -fv ${CEGCCLIB}
${TARGET}-ar q ${CEGCCLIB} cegcc.o

rm -fv cegcc.o
