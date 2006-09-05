#!/bin/sh

BUILD_DIR="build-binutils_cvs_mingwce"
BIN_VERSION=cvs
BASE_DIRECTORY=${PWD}

export TARGET="arm-wince-mingw32"
export PREFIX="/usr/local/mingw32ce"

export CFLAGS='-g3 -O0'

echo ""
echo "BUILDING BINUTILS --------------------------"
echo ""
mkdir -p ${BUILD_DIR} || exit
cd ${BUILD_DIR} || exit
${BASE_DIRECTORY}/binutils-${BIN_VERSION}/configure \
  --prefix=${PREFIX}      \
  --exec-prefix=${PREFIX} \
  --bindir=${PREFIX}/bin  \
  --target=${TARGET}      \
  --disable-nls           \
  --includedir=${PREFIX}/include || exit
make         || exit
make install || exit

cd ${BASE_DIRECTORY} || exit

echo "Done."
echo ""
