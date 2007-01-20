#!/bin/sh

if [ $# -lt 2 ] ; then
  echo "Using defaults:"
  export BASE_DIRECTORY=`readlink -f .`
  export BUILD_DIR=${BASE_DIRECTORY}/build-cegcc
  export PREFIX=/opt/cegcc

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
  BUILD_OPT="$4"
  shift 4
fi

export TARGET="arm-wince-cegcc"
export PATH=${PREFIX}/bin:${PATH}
#export CFLAGS="-g3 -O0"

echo "Building cegcc:"
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


function copy_w32api_headers()
{
    echo ""
    echo "Copying w32api headers. ----------------------"
    echo ""
    echo ""

    mkdir -p ${PREFIX}/${TARGET}/include/w32api
    cp -rf ${BASE_DIRECTORY}/w32api/include/*.h ${PREFIX}/${TARGET}/include/w32api || exit 1
}

function build_dummy_cegccdll()
{
    echo ""
    echo "Building dummy libcegcc.dll.a ----------------------"
    echo ""
    echo ""
    
    pushd ${BASE_DIRECTORY}/cegcc/fakecegccdll || exit 1
    ./install.sh ${PREFIX} || exit 1
    popd || exit 1
}

function build_bootstrap_gcc()
{
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
}

function build_newlib()
{
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

    make || exit 1
    make install || exit 1
    cd ${BASE_DIRECTORY} || exit 1
}

function build_gcc()
{
    echo ""
    echo "Building full gcc. --------------------------"
    echo ""
    echo ""
    
    mkdir -p ${BUILD_DIR}/gcc || exit 1
    pushd ${BUILD_DIR}/gcc || exit 1
    
    ${BASE_DIRECTORY}/gcc/configure    \
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
	|| exit 1


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

    popd || exit 1
}

function build_cegccdll()
{
    echo ""
    echo "Building cegcc.dll --------------------------"
    echo ""
    echo ""

    cd ${BASE_DIRECTORY}/cegcc/cegccdll || exit 1
    make || exit 1
    make install || exit 1
}

function build_cegccthrddll()
{
    echo ""
    echo "Building cegccthrd.dll --------------------------"
    echo ""
    echo ""

    cd ${BASE_DIRECTORY}/cegcc/cegccthrd || exit 1
    make  || exit 1
    make install || exit 1
}

function build_libstdcppdll()
{
    echo ""
    echo "Building libstdc++.dll --------------------------"
    echo ""
    echo ""

    cd ${BASE_DIRECTORY}/cegcc/libstdc++ || exit 1
    make 
    make install
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
    ${TARGET}-gcc -O2 -mwin32  \
           ${STUB_SRC}         \
           -o ${STUB_EXE}      \
           -lwinsock || exit 1
    ${TARGET}-strip ${STUB_EXE} || exit 1

    cd ${BASE_DIRECTORY} || exit 1
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

function build_all()
{
    build_binutils
    build_import_libs
    copy_w32api_headers
    build_dummy_cegccdll
    build_bootstrap_gcc
    build_newlib
    build_gcc
    build_cegccdll
    build_cegccthrddll
    build_libstdcppdll
    build_profile
    build_docs
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
 w32api) copy_w32api_headers ;;
 dummy_cegccdll) build_dummy_cegccdll ;;
 bootstrapgcc) build_bootstrap_gcc ;;
 newlib) build_newlib ;;
 gcc) build_gcc ;;
 cegccdll) build_cegccdll ;;
 cegccthrd) build_cegccthrddll ;;
 libstdc++) build_libstdcppdll ;; 
 gdb) build_gdb ;;
 gdbstub) build_gdbstub ;;
 docs) build_docs ;;
 profile) build_profile ;;
 all) build_all ;;
 *) echo "Please enter a valid build option." ;;
esac

echo ""
echo "Done. --------------------------"
echo ""
echo ""
