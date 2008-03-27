#
# This Makefile converts a 'static' library into a DLL.
#
# /opt/mingw32ce/lib/gcc/arm-mingw32ce/4.3.0/libgcc.a
# ${PREFIX}/lib/gcc/${TARGET}/${GCC_VERSION}/libgcc.a
#
ME=		libgcc
MYDIR=		mingwdll/${ME}
#TARGET?=	arm-mingw32ce
#BUILD_DIR?=	/home/danny/src/cegcc/svn.sf.net/cegcc/trunk/cegcc/src/build-mingw32ce
#PREFIX?=	/opt/mingw32ce
DLLVERSION=	0.55.2
DLL_DIR=        ${PREFIX}/${TARGET}/lib
DEVICE_DIR=     ${PREFIX}/${TARGET}/lib/device

DLL_LIBS=	# -lws2

GCC_VERSION=	`${TARGET}-gcc --version | head -1 | awk '{print $$3;}'`

all::
	mkdir -p ${BUILD_DIR}/${MYDIR}
	${TARGET}-dlltool --output-def ${BUILD_DIR}/${MYDIR}/${ME}.def \
		--export-all ${PREFIX}/lib/gcc/${TARGET}/${GCC_VERSION}/${ME}.a
	cd ${BUILD_DIR}/${MYDIR}; ${TARGET}-gcc --shared \
		-o ${BUILD_DIR}/${MYDIR}/${ME}-${DLLVERSION}.dll \
	 	-Wl,--out-implib,${ME}.dll.a ${BUILD_DIR}/${MYDIR}/${ME}.def \
		${PREFIX}/lib/gcc/${TARGET}/${GCC_VERSION}/${ME}.a ${DLL-LIBS}
	-rm -f ${BUILD_DIR}/${MYDIR}/${ME}-${DLLVERSION}.dll.stripped
	cp ${BUILD_DIR}/${MYDIR}/${ME}-${DLLVERSION}.dll ${BUILD_DIR}/${MYDIR}/${ME}-${DLLVERSION}.dll.stripped
	${TARGET}-strip ${BUILD_DIR}/${MYDIR}/${ME}-${DLLVERSION}.dll.stripped

clean::
	-rm -f ${BUILD_DIR}/${MYDIR}/*.o
	-rm -f ${BUILD_DIR}/${MYDIR}/${ME}*

install::
	mkdir -p ${DLL_DIR}
	cp ${BUILD_DIR}/${MYDIR}/${ME}.def ${DLL_DIR}
	cp ${BUILD_DIR}/${MYDIR}/${ME}-${DLLVERSION}.dll ${DLL_DIR}
