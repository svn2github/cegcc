#!/bin/sh

BUILD_DIR="build-gcc_wince_mingw"
GCC_VERSION=
BASE_DIRECTORY=${PWD}

SOURCE_DIR=${BASE_DIRECTORY}/gcc${GCC_VERSION}

mkdir -p ${BUILD_DIR} || exit

export TARGET="arm-wince-mingw32"
export PREFIX="/usr/local/mingw32ce"
export PATH=${PATH}:${PREFIX}/bin

########################################
# BUILD GCC

echo "BUILDING GCC/MinGW (WinCE)-----------------------"

echo SOURCE_DIR=${SOURCE_DIR}
echo BUILD_DIR=${BUILD_DIR}
echo PREFIX=${PREFIX}

echo ""
echo "Building gcc --------------------------"
echo ""
echo ""

cd ${BUILD_DIR} || exit

${SOURCE_DIR}/configure			 \
  --with-gcc                     \
  --with-gnu-ld                  \
  --with-gnu-as                  \
  --target=${TARGET}             \
  --prefix=${PREFIX}             \
  --enable-threads=win32         \
  --disable-nls                  \
  --enable-languages=c,c++       \
  --disable-clocale              \
  --disable-win32-registry       \
  --disable-multilib             \
  --disable-interwork            \
  --without-newlib               \
  --enable-checking              \
  || exit

make all-gcc || exit
make install-gcc || exit
cd ${BASE_DIRECTORY} || exit

echo ""
echo "DONE --------------------------"
echo ""
echo ""
