#!/bin/sh

if [ $# -lt 3 ] ; then
        echo "usage:"
        echo "$0 [source dir] [build directory] [prefix dir]"
        exit 1
fi

export BASE_DIRECTORY=`readlink -f $1`
export BUILD_DIR=`readlink -f $2`
export PREFIX=`readlink -f $3`
shift 3

export TARGET="arm-wince-pe"

echo "Building cegcc:"
echo "source: ${BASE_DIRECTORY}"
echo "build: ${BUILD_DIR}"
echo "prefix: ${PREFIX}"

export PATH=${PREFIX}/bin:${PATH}

mkdir -p ${BUILD_DIR} || exit 1
mkdir -p ${PREFIX} || exit 1

echo ""
echo "BUILDING BINUTILS --------------------------"
echo ""
mkdir -p ${BUILD_DIR}/binutils || exit 1
cd ${BUILD_DIR}/binutils || exit 1
${BASE_DIRECTORY}/binutils/configure \
  --prefix=${PREFIX}      \
  --exec-prefix=${PREFIX} \
  --bindir=${PREFIX}/bin  \
  --target=${TARGET}      \
  --disable-nls           \
  --includedir=${PREFIX}/include || exit 1
make         || exit 1
make install || exit 1

cd ${BASE_DIRECTORY} || exit 1

##########################################################

echo ""
echo "Building import libs. --------------------------"
echo ""
echo ""

mkdir -p ${PREFIX}/${TARGET}/lib
cd ${BASE_DIRECTORY}/cegcc/importlibs || exit 1
./build.sh ./defs ${PREFIX}/${TARGET}/lib || exit 1

cd ${BASE_DIRECTORY} || exit 1

##########################################################

echo ""
echo "Copying w32api headers. ----------------------"
echo ""
echo ""

mkdir -p ${PREFIX}/${TARGET}/include/w32api
cp -rf w32api/include/*.h ${PREFIX}/${TARGET}/include/w32api

##########################################################

echo ""
echo "Building dummy libcegcc.dll.a ----------------------"
echo ""
echo ""

CEGCCLIB=${PREFIX}/${TARGET}/lib/libcegcc.dll.a
rm -fv ${CEGCCLIB}
${TARGET}-ar q ${CEGCCLIB}

##########################################################

echo ""
echo "Building bootstrap gcc. ----------------------"
echo ""

mkdir -p ${BUILD_DIR}/gcc-bootstrap || exit 1
cd ${BUILD_DIR}/gcc-bootstrap || exit 1

${BASE_DIRECTORY}/gcc/configure		 \
  --with-gcc                     \
  --with-gnu-ld                  \
  --with-gnu-as                  \
  --target=${TARGET}             \
  --prefix=${PREFIX}             \
  --disable-threads              \
  --disable-nls                  \
  --enable-languages=c           \
  --disable-win32-registry       \
  --disable-multilib             \
  --disable-interwork            \
  --without-headers              \
  --enable-checking              \
  || exit 1

make all-gcc        || exit 1
make install-gcc || exit 1
cd ${BASE_DIRECTORY} || exit 1

##############################################################

echo ""
echo "Building newlib. --------------------------"
echo ""
echo ""

mkdir -p ${BUILD_DIR}/newlib || exit 1
cd ${BUILD_DIR}/newlib || exit 1

${BASE_DIRECTORY}/newlib/configure   \
  --target=${TARGET}                 \
  --prefix=${PREFIX}                 \
  || exit 1

make all-target-newlib || exit 1
make install-target-newlib || exit 1
cd ${BASE_DIRECTORY} || exit 1

##############################################################
echo ""
echo "Building full gcc. --------------------------"
echo ""
echo ""

mkdir -p ${BUILD_DIR}/gcc || exit 1
cd ${BUILD_DIR}/gcc || exit 1

${BASE_DIRECTORY}/gcc/configure		 \
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
  --enable-checking              \
  || exit 1

cd ${BUILD_DIR}/gcc || exit 1
make || exit 1
make install || exit 1

##############################################################
echo ""
echo "Building cegcc.dll --------------------------"
echo ""
echo ""

cd ${BASE_DIRECTORY}/cegcc/cegccdll || exit 1
make || exit 1
make install || exit 1

##############################################################
echo ""
echo "Building cegccthrd.dll --------------------------"
echo ""
echo ""

cd ${BASE_DIRECTORY}/cegcc/cegccthrd || exit 1
make  || exit 1
make install || exit 1

##############################################################
echo ""
echo "Building libstdc++.dll --------------------------"
echo ""
echo ""

cd ${BASE_DIRECTORY}/cegcc/libstdc++ || exit 1
make 
make install

##############################################################

echo ""
echo "BUILDING GDB --------------------------"
echo ""
echo ""

mkdir -p ${BUILD_DIR}/gdb || exit 1
cd ${BUILD_DIR}/gdb || exit 1

export CFLAGS="-I${BASE_DIRECTORY}/w32api/include"

${BASE_DIRECTORY}/gdb/configure  \
  --with-gcc                     \
  --with-gnu-ld                  \
  --with-gnu-as                  \
  --target=${TARGET}             \
  --prefix=${PREFIX}             \
  --disable-nls                  \
  --disable-win32-registry       \
  --disable-multilib             \
  --disable-interwork            \
  --enable-checking              \
  || exit

export CFLAGS=

make         || exit 1
make install || exit 1

##############################################################

echo ""
echo "BUILDING GDB stub --------------------------"
echo ""
echo ""

STUB_EXE=${PREFIX}/bin/${TARGET}-stub.exe
STUB_SRC=${BASE_DIRECTORY}/gdb/gdb/wince-stub.c

#pass -static so the stub doesn't depend on cegcc.dll. 
#Useful for debugging cegcc.dll itself.
#Actually, the stub would better be built with -mno-cegcc/arm-wince-mingw32
#To remove the newlib/cegcc.dll dependency, since it mostly uses win32 api.
#Removed for now, as it is giving problems.
${TARGET}-gcc -O2 -mwin32      \
           ${STUB_SRC}         \
           -o ${STUB_EXE}      \
           -lwinsock || exit 1
${TARGET}-strip ${STUB_EXE} || exit 1

cd ${BASE_DIRECTORY} || exit 1

echo ""
echo "Done. --------------------------"
echo ""
echo ""

