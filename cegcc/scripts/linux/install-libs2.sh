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
# Non-standard build
#
if [ $TGT_ARCH = arm-wince-cegcc ]; then
	cd $TOP_SRCDIR/src/cegcc/cegccthrd || exit 1
	make PREFIX=$PREFIX TARGET=$TGT_ARCH install || exit 1
fi
#
cd $TOP_SRCDIR/src/cegcc/libstdc++ || exit 1
make PREFIX=$PREFIX TARGET=$TGT_ARCH install || exit 1
#
# End
#
exit 0
