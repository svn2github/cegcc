#!/bin/sh

BUILD_DIR="build-gcc_mingw32ce"
GCC_VERSION=
BASE_DIRECTORY=${PWD}

SOURCE_DIR=${BASE_DIRECTORY}/gcc${GCC_VERSION}

mkdir -p ${BUILD_DIR} || exit

export TARGET="arm-wince-mingw32"
export PREFIX="/opt/mingw32ce"
export PATH=${PATH}:${PREFIX}/bin

########################################
# BUILD GCC

echo "BUILDING GCC/MinGW (WinCE)-----------------------"

echo SOURCE_DIR=${SOURCE_DIR}
echo BUILD_DIR=${BUILD_DIR}
echo PREFIX=${PREFIX}

echo ""
echo "Building ${TARGET} --------------------------"
echo ""
echo ""

echo "Copying headers... "

mkdir -p ${PREFIX}/${TARGET}/include/
cp -rfp ${BASE_DIRECTORY}/mingw/include/*.h ${PREFIX}/${TARGET}/include/
cp -rfp ${BASE_DIRECTORY}/w32api/include/*.h ${PREFIX}/${TARGET}/include/

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
  --without-newlib               \
  --enable-checking              \
  || exit

#  --disable-clocale              \

make all-gcc || exit
make install-gcc || exit
#make         || exit
#make install || exit
cd ${BASE_DIRECTORY} || exit

echo ""
echo "DONE --------------------------"
echo ""
echo ""
