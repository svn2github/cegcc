#!/bin/bash

DLLTOOL=arm-wince-pe-dlltool
OBJCOPY=arm-wince-pe-objcopy

if [ $# -lt 2 ] ; then
	echo "usage:"
	echo "$0 [input def file] [output directory] [list of symbols to rename]"
	exit 1
fi

def=$1
outputdir=$2
shift 2

lib=$(basename $def)
lib=${lib%.def}

if [ ! -d $outputdir ]
then
	echo "error: output directory \"$outputdir\" not found."
	exit 1
fi

echo "$def -> $outputdir/lib$lib.a"
rm -f $outputdir/lib$lib.a
$DLLTOOL -d $def -l $outputdir/lib$lib.a
if [ ! -f $outputdir/lib$lib.a ]
then
	exit 1
fi

syms="$*"

rensymsfile=__rensyms.tmp
rm -f $rensymsfile
for sym in $syms ; do
	echo "renaming: $sym -> __MS_$sym"
	echo "$sym __MS_$sym" >> $rensymsfile
done

if [ -f $rensymsfile ]
then
	$OBJCOPY --redefine-syms $rensymsfile $outputdir/lib$lib.a
	rm -f $rensymsfile
fi
