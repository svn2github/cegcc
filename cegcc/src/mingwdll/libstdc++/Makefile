#
# This Makefile converts a 'static' library into a DLL.
#
# /opt/mingw32ce/arm-mingw32ce/lib/libstdc++.a
# ${PREFIX}/${TARGET}/lib/libstdc++.a
#
ME=		libstdc++
MYDIR=		mingwdll/${ME}
#TARGET?=	arm-mingw32ce
#BUILD_DIR?=	/home/danny/src/cegcc/svn.sf.net/cegcc/trunk/cegcc/src/build-mingw32ce
#PREFIX?=	/opt/mingw32ce
#BASE_DIRECTORY?=/home/danny/src/cegcc/svn.sf.net/cegcc/trunk/cegcc/src
DLL_DIR=	${PREFIX}/${TARGET}/lib
DEVICE_DIR=	${PREFIX}/${TARGET}/lib/device

DLLVERSION=	`${BASE_DIRECTORY}/mingwdll/get-version`

DLL_LIBS=	# -lws2

GCC_VERSION=	`${TARGET}-gcc --version | head -1 | awk '{print $$3;}'`

all::
	mkdir -p ${BUILD_DIR}/${MYDIR}
	${TARGET}-dlltool --output-def ${BUILD_DIR}/${MYDIR}/${ME}.def \
		--export-all ${PREFIX}/${TARGET}/lib/${ME}.a
	cd ${BUILD_DIR}/${MYDIR}; ${TARGET}-gcc --shared \
		-o ${BUILD_DIR}/${MYDIR}/${ME}-${DLLVERSION}.dll \
	 	-Wl,--out-implib,${ME}.dll.a ${BUILD_DIR}/${MYDIR}/${ME}.def \
		${PREFIX}/${TARGET}/lib/${ME}.a ${DLL-LIBS}
	-rm -f ${BUILD_DIR}/${MYDIR}/${ME}-${DLLVERSION}.dll.stripped
	cp ${BUILD_DIR}/${MYDIR}/${ME}-${DLLVERSION}.dll ${BUILD_DIR}/${MYDIR}/${ME}-${DLLVERSION}.dll.stripped
	${TARGET}-strip ${BUILD_DIR}/${MYDIR}/${ME}-${DLLVERSION}.dll.stripped
	sed \
		-e "s/^dlname=.*\$$/dlname='${ME}-${DLLVERSION}.dll'/" \
		-e "s/^library_names=.*\$$/library_names='${ME}.dll.a'/" \
		<${PREFIX}/${TARGET}/lib/${ME}.la \
		>${BUILD_DIR}/${MYDIR}/${ME}.la

clean::
	-rm -f ${BUILD_DIR}/${MYDIR}/*.o
	-rm -f ${BUILD_DIR}/${MYDIR}/${ME}*

install::
	mkdir -p ${DLL_DIR} ${DEVICE_DIR}
	cp ${BUILD_DIR}/${MYDIR}/${ME}.def ${DLL_DIR}
	cp ${BUILD_DIR}/${MYDIR}/${ME}-${DLLVERSION}.dll ${DLL_DIR}
	cp ${BUILD_DIR}/${MYDIR}/${ME}-${DLLVERSION}.dll.stripped ${DEVICE_DIR}
	rm -f ${DLL_DIR}/${ME}.la
	cp ${BUILD_DIR}/${MYDIR}/${ME}.la ${DLL_DIR}/${ME}.la
