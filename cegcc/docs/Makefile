ARCH=		arm-wince-pe
CC=		${ARCH}-gcc ${CFLAGS} -e_WinMain
WINDRES=	${ARCH}-windres

CFLAGS=		-D_WIN32_IE=0x0400 ${VERBOSE} ${INCLUDE}
LDFLAGS=	-Wl,--major-subsystem-version,4,--minor-subsystem-version,20
PDADIR=		":/storage card/devel"

test5.exe:	test5.o test5.rsc
	${PCC} -o $@ test5.o test5.rsc -lcommctrl ${LDFLAGS}

test5.o:	test5.c
	${PCC} -c $?

test5.rsc:	test5.rc test5.h
	${WINDRES} test5.rc test5.rsc

clean:
	-rm -f *.o *.exe *.rsc

dist:
	for i in *.exe ; do \
		pcp $$i ${PDADIR}/$$i ; \
	done
