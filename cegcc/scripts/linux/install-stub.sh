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
# Not in the MingW target
#
if [ $TGT_ARCH = arm-wince-mingw32ce ]; then
	exit 0
fi

LIBS=""
INCLUDES=""
CFLAGS="-mwin32 ${INCLUDES}"
LDFLAGS="-e WinMainCRTStartup"

mkdir -p ${PREFIX}/bin
cp ${STUB_EXE} ${PREFIX}/bin || exit 1
${TGT_ARCH}-strip ${PREFIX}/bin/${TGT_ARCH}-stub.exe || exit 1
