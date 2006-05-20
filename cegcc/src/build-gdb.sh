#!/bin/sh

BUILD_DIR="build-gdb"
GDB_VERSION=6.3-local
BASE_DIRECTORY=${PWD}

mkdir -p ${BUILD_DIR} || exit

export TARGET="arm-wince-pe"
export PREFIX="/usr/local"
export PATH=${PATH}:${PREFIX}/bin

#stuff for stub
SDKDIR="/cygdrive/d/gcc/wince/sdk"
INCLUDES="-idirafter ${SDKDIR}/Include/Armv4"
LIBS="-L ${SDKDIR}/Lib/Armv4 -nodefaultlibs -nostdlib -lcorelibc -lwinsock -lcoredll"

########################################
# BUILD GDB
echo ""
echo "BUILDING GDB --------------------------"
echo ""
echo ""

cd ${BUILD_DIR} || exit
${BASE_DIRECTORY}/gdb-${GDB_VERSION}/configure \
  --with-gcc                     \
  --with-gnu-ld                  \
  --with-gnu-as                  \
  --target=${TARGET}             \
  --prefix=${PREFIX}             \
  --disable-threads              \
  --disable-nls                  \
  --disable-win32-registry       \
  --disable-multilib             \
  --disable-interwork            \
  --with-newlib                  \
  --enable-checking              \
  || exit

make         || exit
make install || exit

echo ""
echo "BUILDING STUB --------------------"
echo ""

EXE="${PREFIX}/bin/${TARGET}-stub.exe"
SOURCES="${BASE_DIRECTORY}/gdb-${GDB_VERSION}/gdb/wince-stub.c"

${TARGET}-gcc ${SOURCES} -o ${EXE} -e WinMainCRTStartup ${LIBS} ${INCLUDES} || exit
${TARGET}-strip ${EXE} || exit

cd ${BASE_DIRECTORY} || exit

echo ""
echo "DONE --------------------------"
echo ""
echo ""

########################################
