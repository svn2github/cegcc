#!/bin/sh

BUILD_DIR="build-mingw-runtime"
BASE_DIRECTORY=${PWD}
SOURCE_DIR=${BASE_DIRECTORY}/mingw


mkdir -p ${BUILD_DIR} || exit

export TARGET="arm-wince-mingw32"
export PREFIX="/usr/local/mingw32ce"
export PATH=${PATH}:${PREFIX}/bin

#I have this normally set by ccache.
#Must unset them, because mingw being a lib,
#uses $host==$target, and CC instead of CC_FOR_TARGET.
unset CC
unset CXX
#export CFLAGS="-g3 -O0"

echo ""
echo "BUILDING MinGW runtime (WinCE)-----------------------"
echo ""
echo ""

echo SOURCE_DIR=${SOURCE_DIR}
echo BUILD_DIR=${BUILD_DIR}
echo PREFIX=${PREFIX}

cd ${BUILD_DIR} || exit
${SOURCE_DIR}/configure \
  --build=i686-pc-cygwin \
  --host=${TARGET} \
  --target=${TARGET} \
  --prefix=${PREFIX} \
  || exit

make || exit
make install || exit
cd ${BASE_DIRECTORY} || exit

echo ""
echo "DONE --------------------------"
echo ""
echo ""
