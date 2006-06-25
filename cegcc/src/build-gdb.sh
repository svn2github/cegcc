#!/bin/sh

BUILD_DIR="build-gdb"
GDB_VERSION=
BASE_DIRECTORY=${PWD}

mkdir -p ${BUILD_DIR} || exit

export TARGET="arm-wince-pe"
export PREFIX="/usr/local"
export PATH=${PATH}:${PREFIX}/bin

########################################
# BUILD GDB
echo ""
echo "BUILDING GDB --------------------------"
echo ""
echo ""

cd ${BUILD_DIR} || exit
${BASE_DIRECTORY}/gdb${GDB_VERSION}/configure \
  --with-gcc                     \
  --with-gnu-ld                  \
  --with-gnu-as                  \
  --target=${TARGET}             \
  --prefix=${PREFIX}             \
  --disable-nls                  \
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

########################################
