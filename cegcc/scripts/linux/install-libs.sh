#!/bin/sh
#
# Read the settings
#
if [ -r settings.sh ]; then
	. settings.sh
else
	. scripts/linux/settings.sh
fi
cd $BUILD_DIR/libs
#
# Set up a list of DLL proxies to make.
#
. $SCRIPTDIR/list-libs.sh
#
# Create directory
#
mkdir -p $PREFIX/$TGT_ARCH/lib $PREFIX/$MINGW_TGT_ARCH/lib
#
# Loop over the list
#
for i in $INSTALL_PE_TGT
do
	install -m 0755 lib$i.a $PREFIX/$TGT_ARCH/lib
	$TGT_ARCH-ranlib $PREFIX/$TGT_ARCH/lib/lib$i.a
done
for i in $INSTALL_MINGW_TGT
do
	install -m 0755 lib$i.a $PREFIX/$MINGW_TGT_ARCH/lib
	$MINGW_TGT_ARCH-ranlib $PREFIX/$MINGW_TGT_ARCH/lib/lib$i.a
done
# #
# # Set up a libcegcc.a library.
# #
# mkdir /tmp/libs-$$
# cd /tmp/libs-$$
# for i in $LIBS_CEGCC_LIB
# do
# 	$TGT_ARCH-ar x $PREFIX/$TGT_ARCH/lib/lib$i.a
# done
# $TGT_ARCH-ar r $PREFIX/$TGT_ARCH/lib/libcegcc.a *.o
# cd $BUILD_DIR/libs
# rm /tmp/libs-$$/*.o
# $TGT_ARCH-ranlib $PREFIX/$TGT_ARCH/lib/libcegcc.a
#
# End
#
exit 0
