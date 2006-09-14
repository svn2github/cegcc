#!/bin/sh

if [ $# -lt 4 ] ; then
        echo "usage:"
        echo "$0 [source dir] [build directory] [prefix dir] [build_opt]"
        exit 1
fi

export BASE_DIRECTORY=`readlink -f $1`
export BUILD_DIR=`readlink -f $2`
export PREFIX=`readlink -f $3`
shift 3

export TARGET="arm-wince-mingw32"
export BUILD=`sh ${BASE_DIRECTORY}/gcc/config.guess`

echo "Building mingw32:"
echo "source: ${BASE_DIRECTORY}"
echo "build: ${BUILD_DIR}"
echo "prefix: ${PREFIX}"

export PATH=${PREFIX}/bin:${PATH}

mkdir -p ${BUILD_DIR} || exit 1
mkdir -p ${PREFIX} || exit 1

function build_binutils()
{
    echo ""
    echo "BUILDING BINUTILS --------------------------"
    echo ""
    echo ""
    mkdir -p ${BUILD_DIR}/binutils || exit 1
    cd ${BUILD_DIR}/binutils || exit 1
    ${BASE_DIRECTORY}/binutils/configure \
	--prefix=${PREFIX}      \
	--target=${TARGET}      \
	--disable-nls || exit 1

    make         || exit 1
    make install || exit 1
    
    cd ${BASE_DIRECTORY} || exit 1
}    

function build_import_libs()
{
    echo ""
    echo "Building import libs. --------------------------"
    echo ""
    echo ""

    mkdir -p ${PREFIX}/${TARGET}/lib || exit 1
    cd ${BASE_DIRECTORY}/cegcc/importlibs || exit 1
    ./build.sh ./defs ${PREFIX}/${TARGET}/lib || exit 1
    
    cd ${BASE_DIRECTORY} || exit 1
}
    
function build_mingw_fake_runtime()
{
    cd ${BASE_DIRECTORY}/mingw-fake_crt || exit 1
    ./install.sh ${PREFIX} || exit 1
    cd ${BASE_DIRECTORY} || exit 1
}

function copy_headers()
{
    echo ""
    echo "Copying headers... "
    echo ""
    echo ""

    mkdir -p ${PREFIX}/${TARGET}/include/
    cp -rfp ${BASE_DIRECTORY}/mingw/include/*.h ${PREFIX}/${TARGET}/include/
    cp -rfp ${BASE_DIRECTORY}/w32api/include/*.h ${PREFIX}/${TARGET}/include/
}

function build_bootstrap_gcc()
{
    mkdir -p ${BUILD_DIR}/gcc-bootstrap || exit 1
    cd ${BUILD_DIR}/gcc-bootstrap

    ${BASE_DIRECTORY}/gcc/configure	       \
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
	--without-newlib               \
	--enable-checking              \
	|| exit 1
    
    make all-gcc || exit 1
    make install-gcc || exit 1

    cd ${BASE_DIRECTORY} || exit 1
}

function build_mingw_runtime()
{
#I have this normally set by ccache.
#Must unset them, because mingw being a lib,
#uses $host==$target, and CC instead of CC_FOR_TARGET.
    PREV_CC=${CC}
    unset CC

    mkdir -p ${BUILD_DIR}/mingw || exit 1
    cd ${BUILD_DIR}/mingw || exit 1
    ${BASE_DIRECTORY}/mingw/configure \
	--build=${BUILD}              \
	--host=${TARGET}              \
	--target=${TARGET}            \
	--prefix=${PREFIX}            \
	|| exit 1

    make || exit 1
    make install || exit 1

    export CC=${PREV_CC}
    cd ${BASE_DIRECTORY} || exit 1
}

function build_gcc()
{
    mkdir -p ${BUILD_DIR}/gcc || exit 1
    cd ${BUILD_DIR}/gcc || exit

    ${BASE_DIRECTORY}/gcc/configure	\
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

    make || exit 1
    make install || exit 1

    cd ${BASE_DIRECTORY} || exit 1
}

function build_gdb()
{
    echo ""
    echo "BUILDING GDB --------------------------"
    echo ""
    echo ""

    mkdir -p ${BUILD_DIR}/gdb || exit 1
    cd ${BUILD_DIR}/gdb || exit 1

    PREV_CFLAGS=${CFLAGS}
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

    export CFLAGS=${PREV_CFLAGS}

    make         || exit 1
    make install || exit 1
}

function build_gdbstub()
{
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
    ${TARGET}-gcc -O2  \
           ${STUB_SRC}         \
           -o ${STUB_EXE}      \
           -lwinsock || exit 1
    ${TARGET}-strip ${STUB_EXE} || exit 1

    cd ${BASE_DIRECTORY} || exit 1
}

function build_all
{
    build_binutils
    build_import_libs
    build_mingw_fake_runtime
    copy_headers
    build_bootstrap_gcc
    build_mingw_runtime
    build_gcc
#    build_gdb
#    build_gdbstub
}

case "$1" in
 binutils) build_binutils ;;
 importlibs) build_import_libs ;;
 headers) copy_headers ;;
 fakecrt) build_mingw_fake_runtime ;;
 bootstrapgcc) build_bootstrap_gcc ;;
 crt) build_mingw_runtime ;;
 gcc) build_gcc ;;
 gdb) build_gdb ;;
 gdbstub) build_gdbstub ;;
 all) build_all ;;
 *) echo "Please enter a valid build option." ;;
esac

echo ""
echo "DONE --------------------------"
echo ""
echo ""
