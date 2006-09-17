#!/bin/sh
#
# Read the settings
#
if [ -r settings.sh ]; then
	. settings.sh
else
	. scripts/linux/settings.sh
fi

LIBS=""
INCLUDES=""
CFLAGS="-mwin32 ${INCLUDES}"
LDFLAGS="-e WinMainCRTStartup"

mkdir -p ${PREFIX}/bin
cp ${STUB_EXE} ${PREFIX}/bin || exit 1
${TGT_ARCH}-strip ${PREFIX}/bin/${TGT_ARCH}-stub.exe || exit 1
