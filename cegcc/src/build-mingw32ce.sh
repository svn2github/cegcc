#!/bin/sh

########################################
# BUILD MinGW runtime

BUILD_DIR="build-mingw32ce"
MINGW_VERSION=

export BASE_DIRECTORY=${PWD}
export SOURCE_DIR=${BASE_DIRECTORY}/mingw${MINGW_VERSION}
export BUILD=`sh ${SOURCE_DIR}/config.guess`
export HOST="arm-wince-mingw32"
export TARGET=${HOST}
export PREFIX="/opt/mingw32ce"
export PATH=${PATH}:${PREFIX}/bin

#I have this normally set by ccache.
#Must unset them, because mingw being a lib,
#uses $host==$target, and CC instead of CC_FOR_TARGET.
unset CC
unset CXX

#export CFLAGS="-g3 -O0"

echo ""
echo "BUILDING MinGW(CE) runtime for: ${TARGET} -----------"
echo ""
echo ""

echo BUILD=${BUILD}
echo HOST=${HOST}
echo TARGET=${TARGET}

echo SOURCE_DIR=${SOURCE_DIR}
echo BUILD_DIR=${BUILD_DIR}
echo PREFIX=${PREFIX}

cd ${BASE_DIRECTORY}/mingw-fake_crt || exit 1
./install.sh ${PREFIX} || exit 1

mkdir -p ${BUILD_DIR} || exit 1
cd ${BUILD_DIR} || exit 1

${SOURCE_DIR}/configure          \
  --build=${BUILD}               \
  --host=${HOST}                 \
  --target=${TARGET}             \
  --prefix=${PREFIX}             \
  || exit 1

make || exit 1
make install || exit 1
cd ${BASE_DIRECTORY} || exit 1

echo ""
echo "DONE --------------------------"
echo ""
echo ""
