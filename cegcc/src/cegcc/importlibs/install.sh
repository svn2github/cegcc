#!/bin/bash

if [ $# -lt 1 ] ; then
	echo "usage:"
	echo "$0 [prefix directory]"
	exit 1
fi

PREFIX=$1

if [ ! -d $PREFIX ]
then
	echo "error: prefix directory \"$PREFIX\" not found."
	exit 1
fi

cp -rfv lib/* ${PREFIX}/arm-wince-pe/lib
