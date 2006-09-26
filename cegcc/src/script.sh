#!/bin/bash -x

# Based on script from:
# http://cygwin.com/ml/cygwin-apps/2006-05/msg00044.html

TARGET=arm-wince-mingw32ce
PREFIX=/opt/mingw32ce
GCC_VERSION=4.1.0

export PATH=${PREFIX}/bin:${PATH}

srcdir=`readlink -f ./gcc`
builddir=`readlink -f build-mingw32ce`

pushd ${PREFIX}/${TARGET}/lib

LIBVER=$(grep libtool_VERSION= ${srcdir}/libstdc++-v3/configure | sed -e 's/libtool_VERSION=//')
LIBVER_c=$(echo $LIBVER | awk -F: '{print $1}')
LIBVER_r=$(echo $LIBVER | awk -F: '{print $2}')
LIBVER_a=$(echo $LIBVER | awk -F: '{print $3}')
LIBSTDCPP_DLLVER=$(($LIBVER_c - $LIBVER_a))

## no need for a DLLVER for libgcc; it's guaranteed to only add functions,
## never remove or change thier signature.  So it'll always be backwards-compatible.

## It's probably not necessary, but we'll use the LIBSTDCPP_DLLVER for 
## libsupc++ as well (after all, all of libsupc++ appears IN libstdc++
## so if libsupc++ changes in a backwards-non-compatible way, then libstdc++
## will, too -- and the gcc folks will modify its LIBVER.  The downside here
## is that if libsupc++ is unchanged or remains backwards compatible, but
## some other part of libstdc++ changes badly -- we will unnecessarily bump
## libsupc++'s DLLNUM.  'sokay.

pushd ${PREFIX}/lib/gcc/${TARGET}/${GCC_VERSION}
${TARGET}-dlltool --output-def libgcc.def --export-all libgcc.a
${builddir}/gcc/gcc/xgcc -shared -olibgcc.dll \
  -Wl,--out-implib,libgcc.dll.a \
  ./libgcc.def \
  ./libgcc.a
#no versioned implib, no need to ln
popd

${TARGET}-dlltool --output-def libsupc++.def --export-all libsupc++.a
${builddir}/gcc/gcc/xgcc -shared -olibsupc++-${LIBSTDCPP_DLLVER}.dll \
  -Wl,--out-implib,libsupc++-${LIBSTDCPP_DLLVER}.dll.a \
  ./libsupc++.def \
  ./libsupc++.a
rm -f libsupc++.dll.a
ln -s libsupc++-${LIBSTDCPP_DLLVER}.dll.a libsupc++.dll.a

${TARGET}-dlltool --output-def libstdc++.def --export-all libstdc++.a
${builddir}/gcc/gcc/xgcc -shared -olibstdc++-${LIBSTDCPP_DLLVER}.dll \
  -Wl,--out-implib,libstdc++-${LIBSTDCPP_DLLVER}.dll.a \
  ./libstdc++.def \
  ./libstdc++.a

rm -f libstdc++.dll.a
ln -s libstdc++-${LIBSTDCPP_DLLVER}.dll.a libstdc++.dll.a

mkdir -p ${builddir}/device
pushd ${builddir}/device
rm -f libsupc++-${LIBSTDCPP_DLLVER}.dll
rm -f libstdc++-${LIBSTDCPP_DLLVER}.dll
rm -f libgcc.dll

cp -f ${PREFIX}/lib/gcc/${TARGET}/${GCC_VERSION}/libgcc.dll .
cp -f ${PREFIX}/${TARGET}/lib/libsupc++-${LIBSTDCPP_DLLVER}.dll .
cp -f ${PREFIX}/${TARGET}/lib/libstdc++-${LIBSTDCPP_DLLVER}.dll .

${TARGET}-strip libgcc.dll
${TARGET}-strip libsupc++-${LIBSTDCPP_DLLVER}.dll
${TARGET}-strip libstdc++-${LIBSTDCPP_DLLVER}.dll
popd

## MUNGE the .la files
mv libstdc++.la libstdc++.la.SAVE
cat libstdc++.la.SAVE |\
   sed -e "s/^dlname=.*\$/dlname='libstdc++-${LIBSTDCPP_DLLVER}.dll'/" \
       -e "s/^library_names=.*\$/library_names='libstdc++.dll.a'/" >\
   libstdc++.la
rm -f libstdc++.la.SAVE

mv libsupc++.la libsupc++.la.SAVE
cat libsupc++.la.SAVE |\
   sed -e "s/^dlname=.*\$/dlname='libsupc++-${LIBSTDCPP_DLLVER}.dll'/" \
       -e "s/^library_names=.*\$/library_names='libsupc++.dll.a'/" >\
   libsupc++.la
rm -f libsupc++.la.SAVE

popd
