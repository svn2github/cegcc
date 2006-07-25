.SUFFIXES:	.exe .c .h .rc .rsc .C

ARCH=		arm-wince-pe
CC=		${ARCH}-gcc ${CFLAGS}
WINDRES=	${ARCH}-windres
CFLAGS=		-g

.rc.rsc:
		${ARCH}-windres $? $@

.o.exe:
	${ARCH}-gcc ${CFLAGS} -o $@ $?

.c.exe:
	${ARCH}-gcc ${CFLAGS} -o $@ $?

all::	fibo.exe hello-dialog.exe menu-resource.exe

menu-resource.exe:	menu-resource.o menu-resource.rsc
		${CC} -o $@ menu-resource.o menu-resource.rsc -lcommctrl

menu-resource.o:	menu-resource.c
		${CC} -c $?

menu-resource.rsc:	menu-resource.rc menu-resource.h
		${WINDRES} menu-resource.rc menu-resource.rsc

clean:
		-rm -f *.o *.exe *.rsc
