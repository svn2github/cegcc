#!/bin/sh

BUILD_DIR="build-gdb"
GDB_VERSION=6.3-local
BASE_DIRECTORY=${PWD}

export TARGET="arm-wince-pe"
export PREFIX="/usr/local"
export PATH=${PATH}:${PREFIX}/bin

#stuff for stub
SDKDIR="/cygdrive/d/gcc/wince/sdk"
INCLUDES="-idirafter ${SDKDIR}/Include/Armv4"
LIBS="-nodefaultlibs -nostdlib -L ${SDKDIR}/Lib/Armv4 -lcorelibc -lwinsock -lcoredll"

########################################
# BUILD GDB-STUB

echo ""
echo "BUILDING STUB --------------------"
echo ""

EXE="${PREFIX}/bin/${TARGET}-stub.exe"
SOURCES="${BASE_DIRECTORY}/gdb-${GDB_VERSION}/gdb/wince-stub.c"
#SOURCES="${BASE_DIRECTORY}/gdb-${GDB_VERSION}/gdb/mamaich/wince-stub.c"

${TARGET}-gcc -O2 -v -mwin32 ${SOURCES} -o ${EXE} -e WinMainCRTStartup ${LIBS} ${INCLUDES} || exit
${TARGET}-strip ${EXE} || exit

cd ${BASE_DIRECTORY} || exit

echo ""
echo "DONE --------------------------"
echo ""
echo ""

########################################
