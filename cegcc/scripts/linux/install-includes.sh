#!/bin/sh
#
# This won't work, the MinGW build system isn't configured for this.
#
# $TOP_SRCDIR/src/w32api/configure \
#	 --prefix=$PREFIX \
#	--target=$TARGET_ARCH
# make
#
# Instead we're relying on the fact that the w32api/include contains
# a directory tree that we should copy as is.
#
if [ -r settings.sh ]; then
	. settings.sh
else
	. scripts/linux/settings.sh
fi
#
cd $TOP_SRCDIR/src/w32api/include
DIRS=`find . -type d -print | grep -v /.svn`
# echo "Include file directories : [$DIRS]"
for d in $DIRS
do
	mkdir -p $PREFIX/$TARGET_ARCH/include/w32api/$d
done
FILES=`find . -type f -name \*.h -print | grep -v /.svn/ `
# echo "Files : [" $FILES "]"
for i in $FILES
do
	install -m 0644 $i $PREFIX/$TARGET_ARCH/include/w32api/$i
done
exit 0
