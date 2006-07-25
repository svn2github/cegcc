ARCH=		arm-wince-pe
CC=		${ARCH}-gcc ${CFLAGS}
WINDRES=	${ARCH}-windres

CFLAGS=		-D_WIN32_IE=0x0400 ${VERBOSE} ${INCLUDE}
LDFLAGS=	-Wl,--major-subsystem-version,4,--minor-subsystem-version,20
PDADIR=		":/storage card/devel"

menu-resource.exe:	menu-resource.o menu-resource.rsc
	${CC} -o $@ menu-resource.o menu-resource.rsc -lcommctrl ${LDFLAGS}

menu-resource.o:	menu-resource.c
	${CC} -c $?

menu-resource.rsc:	menu-resource.rc menu-resource.h
	${WINDRES} menu-resource.rc menu-resource.rsc

clean:
	-rm -f *.o *.exe *.rsc

dist:
	for i in *.exe ; do \
		pcp $$i ${PDADIR}/$$i ; \
	done
