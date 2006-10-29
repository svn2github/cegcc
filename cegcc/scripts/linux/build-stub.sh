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
CFLAGS="-mwin32 -D_WINSOCKAPI_"
LDFLAGS="-e WinMainCRTStartup -lws2"

${TGT_ARCH}-gcc ${CFLAGS} ${STUB_SRC} -o ${STUB_EXE} ${LDFLAGS} || exit 1

# pcp /home/danny/src/cegcc/svn.berlios.de/cegcc/branches/linux-build/src/gdb/gdb/wince-stub.exe ":/Application data/gdb/wince-stub.exe"
