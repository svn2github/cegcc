#!/usr/bin/bash
#
# Script to extract exports from lib files in given directory.
# The output is a def file for each corresponding lib file
#
# 2006-04-04 pedro alves <pedro_alves@portugalmail.pt>
#

if [ $# -lt 2 ] ; then
	echo "usage:"
	echo "$0 [INPUT_SDK_LIBS_PATH] [OUTPUT_DEF_FILES_PATH]"
	exit 1
fi

SDK_PATH=$1
DEF_PATH=$2

LIBS="$SDK_PATH/*.lib"

LIB_EXT='lib'
DEF_EXT='def'

if [ ! -d $SDK_PATH ]
then
	echo "error: input directory \"$SDK_PATH\" not found."
	exit 1
fi

if [ ! -d $DEF_PATH ]
then
	echo "error: output directory \"$DEF_PATH\" not found."
	exit 1
fi

for lib in $LIBS ; do
	lib2=$(basename $lib)
	dll=${lib2%.$LIB_EXT}
	def=$DEF_PATH/${lib2%$LIB_EXT}$DEF_EXT
	echo "dumping: $lib -> $def"
	dll=`echo $dll | tr a-z A-Z`
	echo -e "LIBRARY $dll\nEXPORTS" > $def
	#export all symbols except the ones with a '?'. Those are c++ symbols with ms mangling.
	arm-wince-pe-nm.exe -A $lib | grep __imp | sed 's/.* __imp_//'|grep -v '?' >> $def
done
