#!/bin/sh

BASE_DIRECTORY=`dirname $0`
BASE_DIRECTORY=`(cd ${BASE_DIRECTORY}; pwd)`
ME=`basename $0`

#
# Initializations.
#
ac_default_prefix="/opt/mingw32ce"
export BUILD_DIR=`pwd`

function usage
{
    cat << _ACEOF

$ME builds the mingw32ce toolchain.

Usage: $0 [OPTIONS] ...

  -h, --help              print this help, then exit
  --prefix=PREFIX         install toolchain in PREFIX
			  [$ac_default_prefix]
  --components=LIST       specify which components to build
                          valid components are: all,binutils,gcc,w32api,mingw,
                          gdb,gdbstub,docs and profile
			  [all]

Report bugs to <cegcc-devel@lists.sourceforge.net>.
_ACEOF

}

ac_prev=
for ac_option
do
  # If the previous option needs an argument, assign it.
  if test -n "$ac_prev"; then
    eval "$ac_prev=\$ac_option"
    ac_prev=
    continue
  fi

  ac_optarg=`expr "x$ac_option" : 'x[^=]*=\(.*\)'`

  case $ac_option in

  -help | --help | --hel | --he | -h)
    usage; exit 0 ;;

  -prefix | --prefix | --prefi | --pref | --pre | --pr | --p)
    ac_prev=prefix ;;
  -prefix=* | --prefix=* | --prefi=* | --pref=* | --pre=* | --pr=* | --p=*)
    prefix=$ac_optarg ;;

  -components | --components | --component | --componen | \
      --compone | --compon | --compo | --comp | --com \
      | --co | --c)
    ac_prev=components ;;
  -components=* | --components=* | --component=* | --componen=* \
      | --compone=* | --compon=* | --compo=* | --comp=* | --com=* \
      | --co=* | --c=*)
    components=$ac_optarg ;;

  -*) { echo "$as_me: error: unrecognized option: $ac_option
Try \`$0 --help' for more information." >&2
   { (exit 1); exit 1; }; }
    ;;

  *=*)
    ac_envvar=`expr "x$ac_option" : 'x\([^=]*\)='`
    # Reject names that are not valid shell variable names.
    expr "x$ac_envvar" : ".*[^_$as_cr_alnum]" >/dev/null &&
      { echo "$as_me: error: invalid variable name: $ac_envvar" >&2
   { (exit 1); exit 1; }; }
    ac_optarg=`echo "$ac_optarg" | sed "s/'/'\\\\\\\\''/g"`
    eval "$ac_envvar='$ac_optarg'"
    export $ac_envvar ;;

  *)
    ;;
  esac
done

if test -n "$ac_prev"; then
  ac_option=--`echo $ac_prev | sed 's/_/-/g'`
  { echo "$as_me: error: missing argument to $ac_option" >&2
   { (exit 1); exit 1; }; }
fi

# Be sure to have absolute paths.
for ac_var in prefix
do
  eval ac_val=$`echo $ac_var`
  case $ac_val in
    [\\/$]* | ?:[\\/]* | NONE | '' ) ;;
    *)  { echo "$as_me: error: expected an absolute directory name for --$ac_var: $ac_val" >&2
   { (exit 1); exit 1; }; };;
  esac
done

if [ "x${prefix}" != "x" ]; then
    export PREFIX="${prefix}"
else
    export PREFIX=${ac_default_prefix}
fi

# Figure out what components where requested to be built.
if test x"${components+set}" != xset; then
    components=all
else
    if test x"${components}" = x ||
	test x"${components}" = xyes;
	then
	echo --components needs at least one argument 1>&2
	exit 1
    fi
fi

# embedded tabs in the sed below -- do not untabify
components=`echo "${components}" | sed -e 's/[ 	,][ 	,]*/,/g' -e 's/,$//'`

# Don't build in source directory, it will clobber things and cleanup is hard.
if [ -d ${BUILD_DIR}/.svn ]; then
	echo "Don't build in a source directory, please create an empty directory and build there."
	echo "Example :"
	echo "  mkdir build-mingw32ce"
	echo "  cd build-mingw32ce"
	echo "  sh ../build-mingw32ce.sh $@"
	exit 1
fi

# Report about options.
echo The following components will be built: ${components}

export TARGET="arm-unknown-mingw32ce"
# export TARGET="arm-wince-mingw32ce"
export BUILD=`sh ${BASE_DIRECTORY}/gcc/config.guess`
export PATH=${PREFIX}/bin:${PATH}

echo "Building mingw32ce:"
echo "source: ${BASE_DIRECTORY}"
echo "building in: ${BUILD_DIR}"
echo "prefix: ${PREFIX}"
echo "components: ${components}"

mkdir -p ${BUILD_DIR} || exit 1
mkdir -p ${PREFIX} || exit 1

build_binutils()
{
    echo ""
    echo "BUILDING BINUTILS --------------------------"
    echo ""
    echo ""
    mkdir -p binutils || exit 1
    cd binutils
    ${BASE_DIRECTORY}/binutils/configure \
	--prefix=${PREFIX}      \
	--target=${TARGET}      \
	--disable-nls || exit 1

    make         || exit 1
    make install || exit 1
    
    cd ${BUILD_DIR}
}    

build_bootstrap_gcc()
{
    mkdir -p gcc-bootstrap || exit 1
    cd gcc-bootstrap

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

    cd ${BUILD_DIR}
}

build_w32api()
{
#I have this normally set by ccache.
#Must unset them, because mingw being a lib,
#uses $host==$target, and CC instead of CC_FOR_TARGET.
    PREV_CC=${CC}
    unset CC

    mkdir -p w32api || exit 1
    cd w32api

    ${BASE_DIRECTORY}/w32api/configure \
	--host=${TARGET}               \
	--prefix=${PREFIX}             \
	|| exit 1

    make || exit 1
    make install || exit 1

    export CC=${PREV_CC}
    cd ${BUILD_DIR}
}

build_mingw_runtime()
{
#I have this normally set by ccache.
#Must unset them, because mingw being a lib,
#uses $host==$target, and CC instead of CC_FOR_TARGET.
    PREV_CC=${CC}
    unset CC

    mkdir -p mingw || exit 1
    cd mingw
    ${BASE_DIRECTORY}/mingw/configure \
	--build=${BUILD}              \
	--host=${TARGET}              \
	--target=${TARGET}            \
	--prefix=${PREFIX}            \
	|| exit 1

    make || exit 1
    make install || exit 1

    export CC=${PREV_CC}
    cd ${BUILD_DIR}
}

build_gcc()
{
    mkdir -p gcc || exit 1
    cd gcc

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

    cd ${BUILD_DIR}
}

build_gdb()
{
    echo ""
    echo "BUILDING GDB --------------------------"
    echo ""
    echo ""

    mkdir -p gdb || exit 1
    cd gdb || exit 1

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

    cd ${BUILD_DIR}
}

build_gdbstub()
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

    cd ${BUILD_DIR}
}

build_docs()
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
    cp src/newlib/COPYING.NEWLIB ${PREFIX} || exit 1

    cd ${BUILD_DIR}
}

build_profile()
{
    echo ""
    echo "BUILDING profiling libraries --------------------------"
    echo ""
    echo ""

    mkdir -p profile || exit 1
    cd profile

    ${BASE_DIRECTORY}/profile/configure  \
	--build=${BUILD}              \
	--host=${TARGET}              \
	--target=${TARGET}            \
	--prefix=${PREFIX}            \
	|| exit

    make         || exit 1
    make install || exit 1

    cd ${BUILD_DIR}
}

build_all()
{
    build_binutils
    build_bootstrap_gcc
    build_mingw_runtime
    build_w32api
    build_gcc
    build_docs
    build_profile
    build_gdb
    build_gdbstub
}

# check for valid options before trying to build them all.
eval "set -- $components"
while [ -n "$1" ]; do
    case $1 in
	binutils | bootstrapgcc | w32api | \
	    mingw | gcc | gdb | gdbstub | \
	    docs | profile | all) 
	    ;;
	*) echo "Please enter a valid build option." ;;
    esac
    shift
done

# now actually try to build them.
eval "set -- $components"
while [ -n "$1" ]; do
    case $1 in
	binutils) build_binutils ;;
	bootstrapgcc) build_bootstrap_gcc ;;
	w32api) build_w32api ;;
	mingw) build_mingw_runtime ;;
	gcc) build_gcc ;;
	gdb) build_gdb ;;
	gdbstub) build_gdbstub ;;
	docs) build_docs ;;
	profile) build_profile ;;
	all) build_all ;;
    esac
    shift
done

echo ""
echo "DONE --------------------------"
echo ""
echo ""
