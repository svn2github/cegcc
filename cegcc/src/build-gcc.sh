#!/bin/sh

BUILD_DIR="build-gcc"
GCC_VERSION=
BASE_DIRECTORY=${PWD}

SOURCE_DIR=${BASE_DIRECTORY}/gcc${GCC_VERSION}

mkdir -p ${BUILD_DIR} || exit

export TARGET="arm-wince-pe"
export PREFIX="/usr/local"
export PATH=${PATH}:${PREFIX}/bin

########################################
# BUILD GCC

echo "BUILDING GCC --------------------------"

echo SOURCE_DIR=${SOURCE_DIR}
echo BUILD_DIR=${BUILD_DIR}
echo PREFIX=${PREFIX}

echo ""
echo "Building dummy libcegcc.dll.a ----------------------"
echo ""
echo ""

LIB=${PREFIX}/arm-wince-pe/lib/libcegcc.dll.a
rm -fv ${LIB}
${TARGET}-ar q ${LIB}

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
  --disable-win32-registry       \
  --disable-multilib             \
  --disable-interwork            \
  --with-newlib                  \
  --enable-checking              \
  || exit

make         || exit
make install || exit
cd ${BASE_DIRECTORY} || exit

echo ""
echo "DONE --------------------------"
echo ""
echo ""
