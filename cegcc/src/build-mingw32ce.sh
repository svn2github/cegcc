#!/bin/sh

if [ $# -lt 2 ] ; then

echo "Using defaults:"
export BASE_DIRECTORY=`readlink -f .`
export BUILD_DIR=${BASE_DIRECTORY}/build-mingw32ce
export PREFIX=/opt/mingw32ce

if [ $# -lt 1 ] ; then
BUILD_OPT="all"
else
BUILD_OPT="$1"
shift
fi

else

export BASE_DIRECTORY=`readlink -f $1`
export BUILD_DIR=`readlink -f $2`
export PREFIX=`readlink -f $3`
BUILD_OPT="$1"
shift 4
fi

export TARGET="arm-wince-mingw32ce"
export BUILD=`sh ${BASE_DIRECTORY}/gcc/config.guess`
export PATH=${PREFIX}/bin:${PATH}
#export CFLAGS="-g3 -O0"

echo "Building mingw32ce:"
echo "source: ${BASE_DIRECTORY}"
echo "build: ${BUILD_DIR}"
echo "prefix: ${PREFIX}"

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
    cp -rfp ${BASE_DIRECTORY}/mingw/include/*.h ${PREFIX}/${TARGET}/include/ || exit 1
    cp -rfp ${BASE_DIRECTORY}/w32api/include/*.h ${PREFIX}/${TARGET}/include/ || exit 1
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

function build_w32api()
{
#I have this normally set by ccache.
#Must unset them, because mingw being a lib,
#uses $host==$target, and CC instead of CC_FOR_TARGET.
    PREV_CC=${CC}
    unset CC

    mkdir -p ${BUILD_DIR}/w32api || exit 1
    pushd ${BUILD_DIR}/w32api || exit 1
    ${BASE_DIRECTORY}/w32api/configure \
	--host=${TARGET}               \
	--prefix=${PREFIX}             \
	|| exit 1

    make || exit 1
    make install || exit 1

    export CC=${PREV_CC}
    popd
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
	--with-headers                 \
	|| exit

# we build libstdc++ as dll, so we don't need this.    
#  --enable-fully-dynamic-string  \

#  --disable-clocale              \

    #
    # Below, the first "make" followed by a file removal, are a workaround
    # for a gcc build bug. The existence of the script causes the first
    # make to fail, the second one should succeed. Therefore, not checking
    # the error code of the first make is intentional.
    #
    make
    rm -f gcc/as
    make || exit 1
    #
    # End workaround
    #
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
	|| exit 1

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

    ${TARGET}-gcc -O2  \
           ${STUB_SRC}         \
           -o ${STUB_EXE}      \
           -lwinsock || exit 1
    ${TARGET}-strip ${STUB_EXE} || exit 1

    cd ${BASE_DIRECTORY} || exit 1
}

function build_docs()
{
    echo ""
    echo "INSTALLING documentation --------------------------"
    echo ""
    echo ""

    mkdir -p ${PREFIX}/share/docs || exit 1
    mkdir -p ${PREFIX}/share/images || exit 1

    cd ${BASE_DIRECTORY}/../docs || exit 1
    tar cf - . | (cd ${PREFIX}/share/docs; tar xf -) || exit 1
    cd ${BASE_DIRECTORY}/../website || exit 1
    tar cf - images | (cd ${PREFIX}/share; tar xf -) || exit 1

    cd ${BASE_DIRECTORY}/.. || exit 1
    cp NEWS README ${PREFIX} || exit 1
    cp src/binutils/COPYING ${PREFIX} || exit 1
    cp src/binutils/COPYING.LIB ${PREFIX} || exit 1
    cp src/binutils/COPYING.NEWLIB ${PREFIX} || exit 1
}

function build_profile()
{
    echo ""
    echo "BUILDING profiling libraries --------------------------"
    echo ""
    echo ""

    mkdir -p ${BUILD_DIR}/profile || exit 1
    cd ${BUILD_DIR}/profile || exit 1

    ${BASE_DIRECTORY}/profile/configure  \
	--build=${BUILD}              \
	--host=${TARGET}              \
	--target=${TARGET}            \
	--prefix=${PREFIX}            \
	|| exit

    make         || exit 1
    make install || exit 1
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
    build_docs
    build_profile
    build_gdb
    build_gdbstub
}

case $BUILD_OPT in
 --help)
        echo "usage:"
        echo "$0 [source dir] [build directory] [prefix dir] [build_opt]"
		;;
 binutils) build_binutils ;;
 importlibs) build_import_libs ;;
 headers) copy_headers ;;
 fakecrt) build_mingw_fake_runtime ;;
 bootstrapgcc) build_bootstrap_gcc ;;
 w32api) build_w32api ;;
 crt) build_mingw_runtime ;;
 gcc) build_gcc ;;
 gdb) build_gdb ;;
 gdbstub) build_gdbstub ;;
 docs) build_docs ;;
 profile) build_profile ;;
 all) build_all ;;
 *) echo "Please enter a valid build option." ;;
esac

echo ""
echo "DONE --------------------------"
echo ""
echo ""
