#!/bin/sh

BUILD_DIR="build-binutils-cvs20050202"
BIN_VERSION=cvs20050202
BASE_DIRECTORY=${PWD}

mkdir -p ${BUILD_DIR} || exit

export TARGET="arm-wince-pe"
export PREFIX="/usr/local"

echo ""
echo "BUILDING BINUTILS --------------------------"
echo ""
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
