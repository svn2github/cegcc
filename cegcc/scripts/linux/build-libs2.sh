#!/bin/sh
#
# Read the settings
#
if [ -r settings.sh ]; then
	. settings.sh
else
	. scripts/linux/settings.sh
fi
#
# Non-standard build, for both libs.
#
if [ $TGT_ARCH = arm-wince-cegcc ]; then
	cd $TOP_SRCDIR/src/cegcc/cegccthrd || exit 1
	make clean TARGET=$TGT_ARCH || exit 1
	make all PREFIX=$PREFIX TARGET=$TGT_ARCH || exit 1
fi
#
cd $TOP_SRCDIR/src/cegcc/libstdc++ || exit 1
make clean TARGET=$TGT_ARCH || exit 1
if [ $TGT_ARCH = arm-wince-mingw32ce ]; then
	TSL="-lcoredll"
else
	TSL="-lcegcc -lcoredll"
fi
make all PREFIX=$PREFIX TARGET=$TGT_ARCH THE_SYSTEM_LIBS=${TSL} || exit 1
#
# End
#
exit 0
