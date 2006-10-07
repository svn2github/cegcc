#!/bin/sh
#
# This won't work, the MinGW build system isn't configured for this.
#
# $TOP_SRCDIR/src/w32api/configure \
#	 --prefix=$PREFIX \
#	--target=$TGT_ARCH
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
	mkdir -p $PREFIX/$TGT_ARCH/include/w32api/$d
#	mkdir -p $PREFIX/$MINGW_TGT_ARCH/include/w32api/$d
done
FILES=`find . -type f -name \*.h -print | grep -v /.svn/ `
# echo "Files : [" $FILES "]"
for i in $FILES
do
	install -m 0644 $i $PREFIX/$TGT_ARCH/include/w32api/$i
#	install -m 0644 $i $PREFIX/$MINGW_TGT_ARCH/include/w32api/$i
done
#
# Install the mingw include directory only in one of the targets
#
if [ $TGT_ARCH = arm-wince-mingw32ce ]; then
	cd $TOP_SRCDIR/src/mingw/include
	DIRS=`find . -type d -print | grep -v /.svn`
	# echo "Include file directories : [$DIRS]"
	for d in $DIRS
	do
		mkdir -p $PREFIX/$TGT_ARCH/include/w32api/$d
	done
	FILES=`find . -type f -name \*.h -print | grep -v /.svn/ `
	# echo "Files : [" $FILES "]"
	for i in $FILES
	do
		install -m 0644 $i $PREFIX/$TGT_ARCH/include/w32api/$i
	done
fi
exit 0
